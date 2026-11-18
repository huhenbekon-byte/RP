package ru.crinstaller

import android.app.Activity
import android.content.ComponentName
import android.content.Intent
import android.content.ServiceConnection
import android.content.pm.PackageManager
import android.graphics.Typeface
import android.net.Uri
import android.os.Bundle
import android.os.Handler
import android.os.IBinder
import android.os.Looper
import android.view.ViewGroup
import android.widget.Button
import android.widget.LinearLayout
import android.widget.ScrollView
import android.widget.TextView
import rikka.shizuku.Shizuku

private const val SHIZUKU_PKG = "moe.shizuku.privileged.api"
private const val REQ_SHIZUKU = 100
private const val REQ_PICK_ZIP = 200

class MainActivity : Activity() {

    private val main = Handler(Looper.getMainLooper())
    private var service: IFileService? = null
    private var busy = false

    private lateinit var statusView: TextView
    private lateinit var logView: TextView
    private lateinit var btnShizuku: Button
    private lateinit var btnInstall: Button
    private lateinit var btnUninstall: Button
    private lateinit var btnPlay: Button

    private val serviceArgs by lazy {
        Shizuku.UserServiceArgs(ComponentName(packageName, FileService::class.java.name))
            .daemon(false)
            .processNameSuffix("files")
            .debuggable(BuildConfig.DEBUG)
            .version(BuildConfig.VERSION_CODE)
    }

    private val connection = object : ServiceConnection {
        override fun onServiceConnected(name: ComponentName?, binder: IBinder?) {
            if (binder != null && binder.pingBinder()) {
                service = IFileService.Stub.asInterface(binder)
                log("Shizuku подключён — доступ к папке игры есть.")
            }
            refresh()
        }

        override fun onServiceDisconnected(name: ComponentName?) {
            service = null
            refresh()
        }
    }

    private val binderReceived = Shizuku.OnBinderReceivedListener {
        main.post {
            if (shizukuState() == ShizukuState.READY) bindFileService()
            refresh()
        }
    }
    private val binderDead = Shizuku.OnBinderDeadListener {
        main.post {
            service = null
            refresh()
        }
    }
    private val permissionResult = Shizuku.OnRequestPermissionResultListener { _, result ->
        main.post {
            if (result == PackageManager.PERMISSION_GRANTED) bindFileService()
            else log("Разрешение для Shizuku не выдано.")
            refresh()
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        buildUi()
        Shizuku.addBinderReceivedListenerSticky(binderReceived)
        Shizuku.addBinderDeadListener(binderDead)
        Shizuku.addRequestPermissionResultListener(permissionResult)
        log(
            "Как пользоваться:\n" +
                "1. Установите GTA San Andreas и запустите её один раз.\n" +
                "2. Установите и запустите Shizuku (через «Отладку по Wi-Fi»).\n" +
                "3. Нажмите «Подключить Shizuku» и разрешите доступ.\n" +
                "4. Нажмите «Установить мод» и выберите архив мода (.zip).\n"
        )
        refresh()
    }

    override fun onResume() {
        super.onResume()
        refresh()
    }

    override fun onDestroy() {
        Shizuku.removeBinderReceivedListener(binderReceived)
        Shizuku.removeBinderDeadListener(binderDead)
        Shizuku.removeRequestPermissionResultListener(permissionResult)
        super.onDestroy()
    }

    private fun buildUi() {
        val pad = (16 * resources.displayMetrics.density).toInt()
        val root = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(pad, pad, pad, pad)
            fitsSystemWindows = true
        }

        root.addView(TextView(this).apply {
            text = "Установщик мода для GTA SA"
            textSize = 22f
            setTypeface(typeface, Typeface.BOLD)
        })

        statusView = TextView(this).apply {
            textSize = 15f
            setPadding(0, pad / 2, 0, pad / 2)
        }
        root.addView(statusView)

        fun button(label: String, onClick: () -> Unit) = Button(this).apply {
            text = label
            textSize = 16f
            isAllCaps = false
            setOnClickListener { onClick() }
            root.addView(this, LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT))
        }

        btnShizuku = button("1. Подключить Shizuku") { onShizukuClick() }
        btnInstall = button("2. Установить мод (выбрать .zip)") { pickZip() }
        btnPlay = button("▶ Играть") { launchGame() }
        btnUninstall = button("Удалить мод (вернуть оригинал)") { runInBackground("Удаляю мод…") { it.uninstall() } }

        logView = TextView(this).apply {
            textSize = 14f
            setTextIsSelectable(true)
            setPadding(0, pad, 0, 0)
        }
        root.addView(logView)

        setContentView(ScrollView(this).apply { addView(root) })
    }

    private enum class ShizukuState { NOT_INSTALLED, NOT_RUNNING, OUTDATED, NO_PERMISSION, READY }

    private fun shizukuState(): ShizukuState = when {
        !Shizuku.pingBinder() -> if (isInstalled(SHIZUKU_PKG)) ShizukuState.NOT_RUNNING else ShizukuState.NOT_INSTALLED
        Shizuku.isPreV11() -> ShizukuState.OUTDATED
        Shizuku.checkSelfPermission() != PackageManager.PERMISSION_GRANTED -> ShizukuState.NO_PERMISSION
        else -> ShizukuState.READY
    }

    private fun isInstalled(pkg: String): Boolean = try {
        packageManager.getPackageInfo(pkg, 0)
        true
    } catch (e: PackageManager.NameNotFoundException) {
        false
    }

    private fun refresh() {
        val gameOk = isInstalled(GAME_PKG)
        val shizuku = shizukuState()
        val modStatus = try {
            service?.status()
        } catch (e: Exception) {
            null
        }

        statusView.text = buildString {
            append(if (gameOk) "✔ GTA San Andreas установлена\n" else "✘ GTA San Andreas не установлена\n")
            append(
                when (shizuku) {
                    ShizukuState.NOT_INSTALLED -> "✘ Shizuku не установлен\n"
                    ShizukuState.NOT_RUNNING -> "✘ Shizuku установлен, но не запущен\n"
                    ShizukuState.OUTDATED -> "✘ Shizuku устарел — обновите его\n"
                    ShizukuState.NO_PERMISSION -> "• Shizuku запущен, нужно разрешение\n"
                    ShizukuState.READY -> if (service != null) "✔ Shizuku подключён\n" else "• Shizuku готов, подключаюсь…\n"
                }
            )
            if (modStatus != null) append("• $modStatus")
        }

        if (shizuku == ShizukuState.READY && service == null) bindFileService()

        val ready = service != null && !busy
        btnShizuku.isEnabled = !busy && service == null
        btnInstall.isEnabled = ready && gameOk
        btnUninstall.isEnabled = ready
        btnPlay.isEnabled = gameOk && !busy
    }

    private fun onShizukuClick() {
        when (shizukuState()) {
            ShizukuState.NOT_INSTALLED -> {
                log("Установите Shizuku из Google Play, запустите его через «Отладку по Wi-Fi» и вернитесь сюда.")
                openUrl("market://details?id=$SHIZUKU_PKG", "https://play.google.com/store/apps/details?id=$SHIZUKU_PKG")
            }
            ShizukuState.NOT_RUNNING -> {
                log("Откройте Shizuku и нажмите «Запустить» в разделе «Запуск через отладку по Wi-Fi», затем вернитесь сюда.")
                packageManager.getLaunchIntentForPackage(SHIZUKU_PKG)?.let { startActivity(it) }
            }
            ShizukuState.OUTDATED -> log("Ваша версия Shizuku слишком старая. Обновите её в Google Play.")
            ShizukuState.NO_PERMISSION -> Shizuku.requestPermission(REQ_SHIZUKU)
            ShizukuState.READY -> bindFileService()
        }
        refresh()
    }

    private fun bindFileService() {
        try {
            Shizuku.bindUserService(serviceArgs, connection)
        } catch (e: Exception) {
            log("Не удалось подключиться к Shizuku: $e")
        }
    }

    private fun pickZip() {
        val intent = Intent(Intent.ACTION_OPEN_DOCUMENT).apply {
            addCategory(Intent.CATEGORY_OPENABLE)
            type = "*/*"
            putExtra(
                Intent.EXTRA_MIME_TYPES,
                arrayOf("application/zip", "application/x-zip-compressed", "application/octet-stream")
            )
        }
        startActivityForResult(intent, REQ_PICK_ZIP)
    }

    @Deprecated("Deprecated in Java")
    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode != REQ_PICK_ZIP || resultCode != RESULT_OK) return
        val uri = data?.data ?: return
        runInBackground("Устанавливаю мод… Это может занять несколько минут, не закрывайте приложение.") { svc ->
            val pfd = contentResolver.openFileDescriptor(uri, "r")
                ?: return@runInBackground "ОШИБКА: не удалось открыть файл."
            pfd.use { svc.install(it) }
        }
    }

    private fun runInBackground(startMessage: String, action: (IFileService) -> String) {
        val svc = service
        if (svc == null) {
            log("Сначала подключите Shizuku.")
            return
        }
        busy = true
        log(startMessage)
        refresh()
        Thread {
            val result = try {
                action(svc)
            } catch (e: Exception) {
                "ОШИБКА: $e"
            }
            main.post {
                busy = false
                log(result)
                refresh()
            }
        }.start()
    }

    private fun launchGame() {
        val intent = packageManager.getLaunchIntentForPackage(GAME_PKG)
        if (intent == null) log("GTA San Andreas не установлена.") else startActivity(intent)
    }

    private fun openUrl(primary: String, fallback: String) {
        try {
            startActivity(Intent(Intent.ACTION_VIEW, Uri.parse(primary)))
        } catch (e: Exception) {
            startActivity(Intent(Intent.ACTION_VIEW, Uri.parse(fallback)))
        }
    }

    private fun log(message: String) {
        logView.append(message.trimEnd() + "\n\n")
    }
}
