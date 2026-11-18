# RP — CRMP Mobile RolePlay (open.mp)

Игровой мод для сервера **CRMP Mobile** на базе **open.mp** (Pawn, MySQL).

## Основа

Код взят из открытого проекта
[rezzylrn/openmp-nusantara-roleplay](https://github.com/rezzylrn/openmp-nusantara-roleplay)
под лицензией **MIT** (см. `LICENSE`, копирайт автора обязательно сохраняем).
Мод модульный (~60 файлов, ~22 тыс. строк), с bcrypt-паролями, MySQL R41,
античитом, работами, фракциями, транспортом, домами, телефоном и инвентарём.

## Структура

```
gamemodes/
  main.pwn        — точка входа
  core/           — БД, авторизация, диалоги, события, локализация
  anticheat/      — античит
  ui/             — HUD, спидометр, прогресс-бар, радиальное меню
  systems/        — экономика, инвентарь, дома, транспорт, магазины и т.д.
  gameplay/       — фракции, работы, телефон, полиция
  commands/       — команды игроков и админов
qawno/include/    — инклуды (open.mp, a_mysql, sscanf2, izcmd, bcrypt, easyDialog)
database.sql      — структура базы MySQL/MariaDB
config.json       — конфиг сервера open.mp
```

## План переделки под CRMP Mobile

1. [x] Перенос основы в репозиторий
2. [ ] Ребрендинг: название сервера, сайт, тексты приветствия
3. [ ] Русский язык вместо индонезийского (кодировка исходников — **Windows-1251**, иначе в клиенте будут «кракозябры»)
4. [ ] Перенос точек (спавн, работы, банки, больница, полиция) на карту CRMP
5. [ ] Транспорт CRMP (ВАЗ, ГАЗ, ПАЗ и т.д.) в автосалоне и работах
6. [ ] Фракции: МВД, ДПС, больница, армия, правительство, СМИ, ОПГ
7. [ ] Адаптация интерфейса под телефон (крупные диалоги, меньше команд с аргументами)

## Сборка

Нужен компилятор [open.mp Pawn compiler 3.10](https://github.com/openmultiplayer/compiler/releases):

```
cd gamemodes
pawncc main.pwn -i../qawno/include -d3
```

Сервер: [open.mp server](https://github.com/openmultiplayer/open.mp/releases) + плагины
`mysql` (R41-4), `samp_bcrypt`, компонент `sscanf`.
