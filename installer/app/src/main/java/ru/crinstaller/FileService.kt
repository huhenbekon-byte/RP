package ru.crinstaller

import android.os.ParcelFileDescriptor
import java.io.BufferedInputStream
import java.io.File
import java.io.FileInputStream
import java.io.FileOutputStream
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale
import java.util.zip.ZipInputStream
import kotlin.system.exitProcess

const val GAME_PKG = "com.rockstargames.gtasa"
const val ROOT = "/storage/emulated/0"
const val DATA_REL = "Android/data/$GAME_PKG"
const val OBB_REL = "Android/obb/$GAME_PKG"
const val BACKUP_DIR = "$ROOT/CRInstaller/backup"

/**
 * Maps a zip entry name to a path relative to [ROOT], or null if the entry must be skipped.
 * The second value is true when the entry was placed by a recognised anchor
 * (a "com.rockstargames.gtasa" or "files" folder inside the archive).
 */
fun mapEntry(raw: String): Pair<String, Boolean>? {
    val parts = raw.replace('\\', '/').split('/').filter { it.isNotEmpty() && it != "." }
    if (parts.isEmpty() || parts.any { it == ".." }) return null

    val fileName = parts.last().lowercase(Locale.ROOT)
    if (fileName.endsWith(".apk") || fileName.endsWith(".exe") || fileName.endsWith(".bat")) return null

    val pkgIdx = parts.indexOf(GAME_PKG)
    if (pkgIdx >= 0) {
        val rest = parts.drop(pkgIdx + 1)
        if (rest.isEmpty()) return null
        val isObb = pkgIdx > 0 && parts[pkgIdx - 1].equals("obb", ignoreCase = true)
        val base = if (isObb) OBB_REL else DATA_REL
        return "$base/${rest.joinToString("/")}" to true
    }

    val filesIdx = parts.indexOf("files")
    if (filesIdx >= 0 && filesIdx < parts.size - 1) {
        return "$DATA_REL/${parts.drop(filesIdx).joinToString("/")}" to true
    }

    // Loose files at the top level of the archive (readme, screenshots) are not game files.
    if (parts.size == 1) return null

    return "$DATA_REL/${parts.joinToString("/")}" to false
}

/**
 * Runs inside the Shizuku process (shell user), which is allowed to write to
 * Android/data and Android/obb of other apps on Android 11+.
 */
class FileService : IFileService.Stub() {

    private val backupRoot = File(BACKUP_DIR)
    private val backupFiles = File(backupRoot, "files")
    private val createdList = File(backupRoot, "created.txt")
    private val marker = File(backupRoot, "installed.txt")

    override fun destroy() {
        exitProcess(0)
    }

    override fun status(): String {
        val data = File(ROOT, DATA_REL)
        return when {
            !data.isDirectory -> "Папка игры не найдена. Запустите GTA SA один раз."
            marker.exists() -> "Мод установлен. " + marker.readText().trim()
            else -> "Мод не установлен."
        }
    }

    override fun install(zip: ParcelFileDescriptor?): String {
        if (zip == null) return "ОШИБКА: архив не передан."
        val data = File(ROOT, DATA_REL)
        if (!data.isDirectory) {
            return "ОШИБКА: папка игры не найдена ($ROOT/$DATA_REL).\n" +
                "Установите GTA San Andreas, запустите её один раз и повторите."
        }
        if (marker.exists()) return "Мод уже установлен. Сначала нажмите «Удалить мод»."

        backupFiles.mkdirs()
        val created = ArrayList<String>()
        val touched = HashSet<String>()
        var written = 0
        var skipped = 0
        var unknown = 0
        var bytes = 0L
        var error: String? = null

        try {
            ZipInputStream(BufferedInputStream(FileInputStream(zip.fileDescriptor), 1 shl 16)).use { zis ->
                while (true) {
                    val entry = zis.nextEntry ?: break
                    if (entry.isDirectory) continue
                    val mapped = mapEntry(entry.name)
                    if (mapped == null) {
                        skipped++
                        continue
                    }
                    val (rel, recognised) = mapped
                    if (!recognised) unknown++

                    val target = File(ROOT, rel)
                    if (touched.add(rel)) {
                        if (target.exists()) {
                            val backup = File(backupFiles, rel)
                            backup.parentFile?.mkdirs()
                            target.copyTo(backup, overwrite = true)
                        } else {
                            created.add(rel)
                        }
                    }
                    target.parentFile?.mkdirs()
                    FileOutputStream(target).use { out -> bytes += zis.copyTo(out, 1 shl 16) }
                    written++
                }
            }
        } catch (e: Exception) {
            error = e.toString()
        } finally {
            createdList.writeText(created.joinToString("\n"))
            val date = SimpleDateFormat("dd.MM.yyyy HH:mm", Locale.ROOT).format(Date())
            marker.writeText("Установлен $date, файлов: $written")
        }

        val sb = StringBuilder()
        if (error != null) {
            sb.append("ОШИБКА при распаковке: ").append(error).append('\n')
            sb.append("Часть файлов уже записана. Нажмите «Удалить мод», чтобы вернуть оригинал.\n")
        } else {
            sb.append("ГОТОВО! Мод установлен.\n")
        }
        sb.append("Записано файлов: $written (${bytes / (1024 * 1024)} МБ)\n")
        sb.append("Новых файлов: ${created.size}, заменено с резервной копией: ${touched.size - created.size}\n")
        if (skipped > 0) sb.append("Пропущено (readme, apk, exe и т.п.): $skipped\n")
        if (unknown > 0) {
            sb.append("Внимание: $unknown файлов без папки com.rockstargames.gtasa — положены в корень папки игры.\n")
        }
        if (written == 0) sb.append("В архиве не нашлось файлов игры. Проверьте, что это архив мода (.zip).\n")
        return sb.toString()
    }

    override fun uninstall(): String {
        if (!backupRoot.exists()) return "Мод не установлен — удалять нечего."
        var removed = 0
        var restored = 0
        if (createdList.exists()) {
            createdList.readLines().filter { it.isNotBlank() }.forEach { rel ->
                if (File(ROOT, rel).delete()) removed++
            }
        }
        if (backupFiles.exists()) {
            backupFiles.walkTopDown().filter { it.isFile }.forEach { f ->
                val target = File(ROOT, f.relativeTo(backupFiles).path)
                target.parentFile?.mkdirs()
                f.copyTo(target, overwrite = true)
                restored++
            }
        }
        backupRoot.deleteRecursively()
        File(BACKUP_DIR).parentFile?.delete()
        return "Мод удалён.\nУдалено новых файлов: $removed\nВосстановлено оригинальных: $restored\n"
    }
}
