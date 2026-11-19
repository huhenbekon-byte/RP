#if defined _INC_rp_wild_west
	#endinput
#endif
#define _INC_rp_wild_west

/*
==============================================================================
 wild_west.inc — режим «Дикий Запад» (Фронтир)

 Собственный вестерн-режим поверх RP-мода:
   * дуэли на револьверах со ставкой          /duel, /duelaccept, /dueldecline
   * награды за голову и розыск               /bounty, /bounties, /payfine
   * честь (репутация) и цены у торговцев      /honor
   * лошадь из конюшни (вызов свистом)         /horse
   * охота и фактория (шкуры, мясо)            /hunt, /buyrifle, /sellpelts
   * ограбление дилижанса (событие сервера)   /robcoach, /wwcoach (админ)
   * «Меткий глаз» — +50% урона на 10 секунд   /deadeye
   * справка                                   /ww (/zapad)

 Все координаты ниже — точки на карте SA (округ Боун, Форт-Карсон).
 При переносе на карту CRMP поменяйте только блок #define с координатами.

 ВАЖНО: файл хранится в кодировке Windows-1251, как и весь русский текст мода.
==============================================================================
*/

// ---------------------------------------------------------------------------
// Настройки
// ---------------------------------------------------------------------------

// Модель «лошади». В стандартной SA лошадей нет — по умолчанию квадроцикл.
// Если на сервере есть кастомная модель лошади (AddSimpleModel / CRMP) —
// впишите её ID сюда.
#define WW_HORSE_MODEL          471
#define WW_HORSE_PRICE          50
#define WW_HORSE_CALL_COOLDOWN  60

// Дилижанс (транспорт события ограбления)
#define WW_COACH_MODEL          478     // Walton — старый грузовичок
#define WW_COACH_INTERVAL       (25*60) // раз в 25 минут
#define WW_COACH_LIFETIME       (8*60)  // стоит на дороге 8 минут
#define WW_COACH_ROB_TIME       15      // секунд на ограбление
#define WW_COACH_REWARD_MIN     900
#define WW_COACH_REWARD_RAND    700
#define WW_COACH_WANTED         400     // розыск за ограбление
#define WW_COACH_MIN_PLAYERS    2       // событие только если онлайн >= N

// Дуэли
#define WW_DUEL_MAX_STAKE       5000
#define WW_DUEL_RANGE           20.0
#define WW_DUEL_INVITE_TIMEOUT  30
#define WW_DUEL_FIGHT_TIME      60
#define WW_DUEL_WEAPON          24      // Desert Eagle как револьвер
#define WW_DUEL_AMMO            6

// Розыск и награды
#define WW_MURDER_WANTED        250     // розыск за убийство невиновного
#define WW_BOUNTY_MIN           100
#define WW_BOUNTY_MAX           100000

// Охота и фактория
#define WW_RIFLE_PRICE          350
#define WW_RIFLE_AMMO           20
#define WW_PELT_PRICE           90
#define WW_MEAT_PRICE           35
#define WW_HUNT_TIME            8
#define WW_HUNT_COOLDOWN        20
#define WW_HUNT_RANGE           250.0

// Меткий глаз
#define WW_DEADEYE_TIME         10
#define WW_DEADEYE_COOLDOWN     180

// ---------------------------------------------------------------------------
// Координаты (SA: Форт-Карсон и окрестности)
// ---------------------------------------------------------------------------
#define WW_SHERIFF_X            -217.0
#define WW_SHERIFF_Y            979.0
#define WW_SHERIFF_Z            19.5

#define WW_STABLE_X             -260.0
#define WW_STABLE_Y             1160.0
#define WW_STABLE_Z             19.7

#define WW_HORSE_SPAWN_X        -255.0
#define WW_HORSE_SPAWN_Y        1165.0
#define WW_HORSE_SPAWN_Z        20.2
#define WW_HORSE_SPAWN_A        90.0

#define WW_TRADER_X             -180.0
#define WW_TRADER_Y             1030.0
#define WW_TRADER_Z             19.7

#define WW_HUNT_X               -500.0
#define WW_HUNT_Y               1500.0
#define WW_HUNT_Z               30.0

static const Float:gWWCoachSpots[][4] = {
	{-150.0, 1100.0, 20.5, 0.0},
	{-220.0, 1190.0, 20.5, 90.0},
	{-60.0,  1050.0, 20.5, 180.0},
	{-300.0, 1050.0, 20.5, 270.0}
};

#define WW_MAPICON_SHERIFF      90
#define WW_MAPICON_STABLE       91
#define WW_MAPICON_TRADER       92
#define WW_MAPICON_HUNT         93

#define COLOR_WW                0xD2A066FF  // «песочный» цвет вестерна
#define WW_TAG                  "{D2A066}[ФРОНТИР]{FFFFFF} "

// ---------------------------------------------------------------------------
// Состояние
// ---------------------------------------------------------------------------
enum {
	WW_DUEL_NONE = 0,
	WW_DUEL_COUNTDOWN,
	WW_DUEL_FIGHT
};

static bool:gWWLoaded[MAX_PLAYERS];
static gWWBounty[MAX_PLAYERS];      // награда, назначенная игроками (деньги уже внесены)
static gWWWanted[MAX_PLAYERS];      // розыск от шерифа (оплачивается из кармана преступника)
static gWWHonor[MAX_PLAYERS];       // -100 .. 100
static gWWPelts[MAX_PLAYERS];
static gWWMeat[MAX_PLAYERS];
static gWWClaimed[MAX_PLAYERS];     // сколько голов сдано

static gWWHorse[MAX_PLAYERS] = {INVALID_VEHICLE_ID, ...};
static gWWHorseCallTick[MAX_PLAYERS];

static gWWDeadEyeEnd[MAX_PLAYERS];
static gWWDeadEyeReady[MAX_PLAYERS];

static gWWHuntReady[MAX_PLAYERS];
static gWWHuntTrack[MAX_PLAYERS];

static gWWInviteFrom[MAX_PLAYERS] = {INVALID_PLAYER_ID, ...};
static gWWInviteStake[MAX_PLAYERS];
static gWWInviteTick[MAX_PLAYERS];

static gWWDuelState[MAX_PLAYERS];
static gWWDuelOpponent[MAX_PLAYERS] = {INVALID_PLAYER_ID, ...};
static gWWDuelStake[MAX_PLAYERS];
static gWWDuelTimer[MAX_PLAYERS];
static bool:gWWDuelHadGun[MAX_PLAYERS];

static gWWCoachVeh = INVALID_VEHICLE_ID;
static Text3D:gWWCoachLabel = INVALID_3DTEXT_ID;
static gWWCoachNext;
static gWWCoachExpire;
static gWWCoachRobber = INVALID_PLAYER_ID;
static gWWCoachRobTick;

forward OnWWPlayerLoaded(playerid, charid);
forward OnWWHuntTracked(playerid, extra);
forward OnWWCoachRobbed(playerid, extra);

// ---------------------------------------------------------------------------
// Инициализация и БД
// ---------------------------------------------------------------------------
stock WildWest_Init()
{
	if (gDBReady) {
		mysql_tquery(gDB, "CREATE TABLE IF NOT EXISTS ww_players (\
			char_id INT PRIMARY KEY, \
			bounty INT DEFAULT 0, \
			wanted INT DEFAULT 0, \
			honor INT DEFAULT 0, \
			pelts INT DEFAULT 0, \
			meat INT DEFAULT 0, \
			claimed INT DEFAULT 0)");
	}

	CreatePickup(1239, 1, WW_SHERIFF_X, WW_SHERIFF_Y, WW_SHERIFF_Z);
	CreatePickup(1239, 1, WW_STABLE_X, WW_STABLE_Y, WW_STABLE_Z);
	CreatePickup(1239, 1, WW_TRADER_X, WW_TRADER_Y, WW_TRADER_Z);

	Create3DTextLabel("{D2A066}[ ОФИС ШЕРИФА ]\n{FFFFFF}/bounties — доска «Разыскивается»\n/payfine — погасить розыск",
		COLOR_WW, WW_SHERIFF_X, WW_SHERIFF_Y, WW_SHERIFF_Z + 0.8, 15.0, 0, false);
	Create3DTextLabel("{D2A066}[ КОНЮШНЯ ]\n{FFFFFF}/horse — взять лошадь ($50)",
		COLOR_WW, WW_STABLE_X, WW_STABLE_Y, WW_STABLE_Z + 0.8, 15.0, 0, false);
	Create3DTextLabel("{D2A066}[ ФАКТОРИЯ ]\n{FFFFFF}/buyrifle — винтовка ($350)\n/sellpelts — продать шкуры и мясо",
		COLOR_WW, WW_TRADER_X, WW_TRADER_Y, WW_TRADER_Z + 0.8, 15.0, 0, false);
	Create3DTextLabel("{2ECC71}[ ОХОТНИЧЬИ УГОДЬЯ ]\n{FFFFFF}/hunt — выследить зверя (нужна винтовка или дробовик)",
		COLOR_GREEN, WW_HUNT_X, WW_HUNT_Y, WW_HUNT_Z + 0.8, 40.0, 0, false);

	gWWCoachNext = gettime() + WW_COACH_INTERVAL;
	print("[WILD WEST] Режим «Дикий Запад» загружен.");
	return 1;
}

stock WW_ResetPlayer(playerid)
{
	gWWLoaded[playerid] = false;
	gWWBounty[playerid] = 0;
	gWWWanted[playerid] = 0;
	gWWHonor[playerid] = 0;
	gWWPelts[playerid] = 0;
	gWWMeat[playerid] = 0;
	gWWClaimed[playerid] = 0;
	gWWHorseCallTick[playerid] = 0;
	gWWDeadEyeEnd[playerid] = 0;
	gWWDeadEyeReady[playerid] = 0;
	gWWHuntReady[playerid] = 0;
	gWWHuntTrack[playerid] = 0;
	gWWInviteFrom[playerid] = INVALID_PLAYER_ID;
	gWWInviteStake[playerid] = 0;
	gWWInviteTick[playerid] = 0;
	gWWDuelState[playerid] = WW_DUEL_NONE;
	gWWDuelOpponent[playerid] = INVALID_PLAYER_ID;
	gWWDuelStake[playerid] = 0;
	gWWDuelTimer[playerid] = 0;
	gWWDuelHadGun[playerid] = false;
	return 1;
}

// Вызывается из OnCharacterLoaded после успешного входа
stock WW_LoadPlayer(playerid)
{
	WW_ResetPlayer(playerid);
	if (!gDBReady) {
		gWWLoaded[playerid] = true;
		return 1;
	}
	new query[128];
	mysql_format(gDB, query, sizeof(query),
		"SELECT bounty, wanted, honor, pelts, meat, claimed FROM ww_players WHERE char_id = %d LIMIT 1",
		gPlayerData[playerid][pID]);
	mysql_tquery(gDB, query, "OnWWPlayerLoaded", "ii", playerid, gPlayerData[playerid][pID]);
	return 1;
}

public OnWWPlayerLoaded(playerid, charid)
{
	// защита от гонки: игрок мог выйти, а слот занять другой персонаж
	if (!IsPlayerConnected(playerid) || !gPlayerData[playerid][pLoggedIn]) return 0;
	if (gPlayerData[playerid][pID] != charid) return 0;

	if (cache_num_rows() > 0) {
		cache_get_value_name_int(0, "bounty", gWWBounty[playerid]);
		cache_get_value_name_int(0, "wanted", gWWWanted[playerid]);
		cache_get_value_name_int(0, "honor", gWWHonor[playerid]);
		cache_get_value_name_int(0, "pelts", gWWPelts[playerid]);
		cache_get_value_name_int(0, "meat", gWWMeat[playerid]);
		cache_get_value_name_int(0, "claimed", gWWClaimed[playerid]);
	}
	gWWLoaded[playerid] = true;

	SetPlayerMapIcon(playerid, WW_MAPICON_SHERIFF, WW_SHERIFF_X, WW_SHERIFF_Y, WW_SHERIFF_Z, 30, 0, MAPICON_LOCAL);
	SetPlayerMapIcon(playerid, WW_MAPICON_STABLE, WW_STABLE_X, WW_STABLE_Y, WW_STABLE_Z, 55, 0, MAPICON_LOCAL);
	SetPlayerMapIcon(playerid, WW_MAPICON_TRADER, WW_TRADER_X, WW_TRADER_Y, WW_TRADER_Z, 6, 0, MAPICON_LOCAL);
	SetPlayerMapIcon(playerid, WW_MAPICON_HUNT, WW_HUNT_X, WW_HUNT_Y, WW_HUNT_Z, 56, 0, MAPICON_LOCAL);

	SendClientMessage(playerid, COLOR_WW, WW_TAG "Добро пожаловать на Фронтир! Справка по режиму: /ww");
	if (WW_GetTotalBounty(playerid) > 0) {
		SendRP(playerid, COLOR_ERROR, WW_TAG "Вы в розыске! Награда за вашу голову: $%d.", WW_GetTotalBounty(playerid));
	}
	return 1;
}

stock WW_SavePlayer(playerid)
{
	if (!gDBReady || !gWWLoaded[playerid]) return 0;
	if (!gPlayerData[playerid][pLoggedIn] || gPlayerData[playerid][pID] <= 0) return 0;

	new query[320];
	mysql_format(gDB, query, sizeof(query),
		"INSERT INTO ww_players (char_id, bounty, wanted, honor, pelts, meat, claimed) VALUES (%d, %d, %d, %d, %d, %d, %d) \
ON DUPLICATE KEY UPDATE bounty = VALUES(bounty), wanted = VALUES(wanted), honor = VALUES(honor), \
pelts = VALUES(pelts), meat = VALUES(meat), claimed = VALUES(claimed)",
		gPlayerData[playerid][pID], gWWBounty[playerid], gWWWanted[playerid], gWWHonor[playerid],
		gWWPelts[playerid], gWWMeat[playerid], gWWClaimed[playerid]);
	mysql_tquery(gDB, query, "", "");
	return 1;
}

stock WW_OnPlayerDisconnect(playerid)
{
	// побег с дуэли = поражение
	if (gWWDuelState[playerid] != WW_DUEL_NONE) {
		new opp = gWWDuelOpponent[playerid];
		if (opp != INVALID_PLAYER_ID && IsPlayerConnected(opp)) {
			SendClientMessage(opp, COLOR_WW, WW_TAG "Противник сбежал с дуэли. Победа ваша!");
			WW_FinishDuel(opp, playerid);
		} else {
			WW_ClearDuel(playerid);
		}
	}
	// отменить чужие приглашения от этого игрока
	for (new i = 0; i < MAX_PLAYERS; i++) {
		if (gWWInviteFrom[i] == playerid) gWWInviteFrom[i] = INVALID_PLAYER_ID;
	}
	if (gWWCoachRobber == playerid) gWWCoachRobber = INVALID_PLAYER_ID;

	WW_DestroyHorse(playerid);
	WW_SavePlayer(playerid);
	WW_ResetPlayer(playerid);
	return 1;
}

// ---------------------------------------------------------------------------
// Вспомогательное
// ---------------------------------------------------------------------------
stock WW_GetTotalBounty(playerid)
{
	return gWWBounty[playerid] + gWWWanted[playerid];
}

stock WW_AddHonor(playerid, delta)
{
	gWWHonor[playerid] += delta;
	if (gWWHonor[playerid] > 100) gWWHonor[playerid] = 100;
	if (gWWHonor[playerid] < -100) gWWHonor[playerid] = -100;
	return gWWHonor[playerid];
}

stock WW_GetHonorTitle(playerid, title[], size = sizeof(title))
{
	new h = gWWHonor[playerid];
	if (h >= 90)       format(title, size, "Легенда Фронтира");
	else if (h >= 50)  format(title, size, "Уважаемый гражданин");
	else if (h >= 0)   format(title, size, "Простой поселенец");
	else if (h > -50)  format(title, size, "Сомнительный тип");
	else               format(title, size, "Отпетый бандит");
	return 1;
}

// Честь влияет на цены: уважаемым скидка 10%, бандитам наценка 20%
stock WW_BuyPrice(playerid, base)
{
	if (gWWHonor[playerid] >= 50) return (base * 90) / 100;
	if (gWWHonor[playerid] <= -50) return (base * 120) / 100;
	return base;
}

stock WW_SellPrice(playerid, base)
{
	if (gWWHonor[playerid] >= 50) return (base * 110) / 100;
	if (gWWHonor[playerid] <= -50) return (base * 85) / 100;
	return base;
}

stock bool:WW_CanAct(playerid)
{
	if (!gPlayerData[playerid][pLoggedIn]) return false;
	if (gPlayerData[playerid][pInjured] || gPlayerData[playerid][pCuffed]) {
		SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Вы ранены или связаны.");
		return false;
	}
	return true;
}

stock bool:WW_IsSheriff(playerid)
{
	return (gPlayerData[playerid][pFaction] == FACTION_POLICE && gPlayerData[playerid][pDuty]);
}

stock WW_AlertSheriffs(const msg[])
{
	for (new i = 0; i < MAX_PLAYERS; i++) {
		if (!IsPlayerConnected(i) || !gPlayerData[i][pLoggedIn]) continue;
		if (gPlayerData[i][pFaction] != FACTION_POLICE) continue;
		SendClientMessage(i, COLOR_DEPARTMENT, msg);
	}
	return 1;
}

stock WW_Broadcast(const msg[])
{
	for (new i = 0; i < MAX_PLAYERS; i++) {
		if (IsPlayerConnected(i) && gPlayerData[i][pLoggedIn]) SendClientMessage(i, COLOR_WW, msg);
	}
	return 1;
}

stock WW_CountOnline()
{
	new n = 0;
	for (new i = 0; i < MAX_PLAYERS; i++) {
		if (IsPlayerConnected(i) && gPlayerData[i][pLoggedIn]) n++;
	}
	return n;
}

stock bool:WW_IsVehicleOccupied(vehicleid)
{
	for (new i = 0; i < MAX_PLAYERS; i++) {
		if (IsPlayerConnected(i) && GetPlayerVehicleID(i) == vehicleid) return true;
	}
	return false;
}

stock bool:WW_HasHuntingWeapon(playerid)
{
	new WEAPON:w = GetPlayerWeapon(playerid);
	return (w == WEAPON_RIFLE || w == WEAPON_SNIPER || w == WEAPON_SHOTGUN);
}

// ---------------------------------------------------------------------------
// Ежесекундный тик (из RP_Tick)
// ---------------------------------------------------------------------------
stock WW_Tick()
{
	new now = gettime();

	for (new i = 0; i < MAX_PLAYERS; i++) {
		if (!IsPlayerConnected(i) || !gPlayerData[i][pLoggedIn]) continue;

		// истёкшие приглашения на дуэль
		if (gWWInviteFrom[i] != INVALID_PLAYER_ID && now - gWWInviteTick[i] > WW_DUEL_INVITE_TIMEOUT) {
			gWWInviteFrom[i] = INVALID_PLAYER_ID;
		}

		// окончание «Меткого глаза»
		if (gWWDeadEyeEnd[i] != 0 && now >= gWWDeadEyeEnd[i]) {
			gWWDeadEyeEnd[i] = 0;
			SendClientMessage(i, COLOR_GREY, WW_TAG "«Меткий глаз» рассеялся.");
		}

		WW_DuelTick(i);
	}

	WW_CoachTick(now);
	return 1;
}

// ---------------------------------------------------------------------------
// Дуэли
// ---------------------------------------------------------------------------
stock WW_ClearDuel(playerid)
{
	if (IsPlayerConnected(playerid)) {
		TogglePlayerControllable(playerid, true);
		if (!gWWDuelHadGun[playerid]) {
			gACData[playerid][acHasWeapon][WW_DUEL_WEAPON] = false;
			SetPlayerAmmo(playerid, WEAPON:WW_DUEL_WEAPON, 0);
		}
	}
	gWWDuelState[playerid] = WW_DUEL_NONE;
	gWWDuelOpponent[playerid] = INVALID_PLAYER_ID;
	gWWDuelStake[playerid] = 0;
	gWWDuelTimer[playerid] = 0;
	gWWDuelHadGun[playerid] = false;
	return 1;
}

// winner получает обе ставки
stock WW_FinishDuel(winner, loser)
{
	new pot = gWWDuelStake[winner] + gWWDuelStake[loser];
	if (pot > 0) GivePlayerMoneyEx(winner, pot, "выигрыш дуэли");
	WW_AddHonor(winner, 2);
	GivePlayerExp(winner, 15, "победа на дуэли");

	new msg[160];
	format(msg, sizeof(msg), WW_TAG "%s одолел %s на дуэли и забрал банк $%d.",
		gPlayerData[winner][pName], gPlayerData[loser][pName], pot);
	WW_Broadcast(msg);

	WW_ClearDuel(winner);
	WW_ClearDuel(loser);
	return 1;
}

stock WW_DrawDuel(a, b)
{
	if (gWWDuelStake[a] > 0) GivePlayerMoneyEx(a, gWWDuelStake[a], "возврат ставки дуэли");
	if (gWWDuelStake[b] > 0) GivePlayerMoneyEx(b, gWWDuelStake[b], "возврат ставки дуэли");
	SendClientMessage(a, COLOR_WW, WW_TAG "Время дуэли вышло — ничья. Ставки возвращены.");
	SendClientMessage(b, COLOR_WW, WW_TAG "Время дуэли вышло — ничья. Ставки возвращены.");
	WW_ClearDuel(a);
	WW_ClearDuel(b);
	return 1;
}

stock WW_DuelTick(playerid)
{
	if (gWWDuelState[playerid] == WW_DUEL_NONE) return 0;
	new opp = gWWDuelOpponent[playerid];
	// тик обрабатывает только игрок с меньшим ID, чтобы не считать дважды
	if (opp == INVALID_PLAYER_ID || !IsPlayerConnected(opp)) {
		WW_ClearDuel(playerid);
		return 0;
	}
	if (playerid > opp) return 0;

	if (gWWDuelState[playerid] == WW_DUEL_COUNTDOWN) {
		gWWDuelTimer[playerid]--;
		gWWDuelTimer[opp] = gWWDuelTimer[playerid];
		if (gWWDuelTimer[playerid] > 0) {
			new txt[16];
			format(txt, sizeof(txt), "~y~%d", gWWDuelTimer[playerid]);
			GameTextForPlayer(playerid, txt, 1000, 3);
			GameTextForPlayer(opp, txt, 1000, 3);
			PlayerPlaySound(playerid, 1056, 0.0, 0.0, 0.0);
			PlayerPlaySound(opp, 1056, 0.0, 0.0, 0.0);
		} else {
			gWWDuelState[playerid] = WW_DUEL_FIGHT;
			gWWDuelState[opp] = WW_DUEL_FIGHT;
			gWWDuelTimer[playerid] = WW_DUEL_FIGHT_TIME;
			gWWDuelTimer[opp] = WW_DUEL_FIGHT_TIME;
			TogglePlayerControllable(playerid, true);
			TogglePlayerControllable(opp, true);
			GameTextForPlayer(playerid, "~r~FIRE!", 1500, 3);
			GameTextForPlayer(opp, "~r~FIRE!", 1500, 3);
			PlayerPlaySound(playerid, 1057, 0.0, 0.0, 0.0);
			PlayerPlaySound(opp, 1057, 0.0, 0.0, 0.0);
		}
		return 1;
	}

	if (gWWDuelState[playerid] == WW_DUEL_FIGHT) {
		gWWDuelTimer[playerid]--;
		gWWDuelTimer[opp] = gWWDuelTimer[playerid];
		if (gWWDuelTimer[playerid] <= 0) {
			WW_DrawDuel(playerid, opp);
		}
	}
	return 1;
}

stock WW_StartDuel(a, b, stake)
{
	if (stake > 0) {
		if (!TakePlayerMoneyEx(a, stake, "ставка на дуэль")) return 0;
		if (!TakePlayerMoneyEx(b, stake, "ставка на дуэль")) {
			GivePlayerMoneyEx(a, stake, "возврат ставки дуэли");
			return 0;
		}
	}

	new pair[2];
	pair[0] = a;
	pair[1] = b;
	for (new k = 0; k < 2; k++) {
		new p = pair[k];
		gWWDuelState[p] = WW_DUEL_COUNTDOWN;
		gWWDuelOpponent[p] = pair[1 - k];
		gWWDuelStake[p] = stake;
		gWWDuelTimer[p] = 4;
		gWWDuelHadGun[p] = gACData[p][acHasWeapon][WW_DUEL_WEAPON];
		gWWInviteFrom[p] = INVALID_PLAYER_ID;

		AC_GiveWeapon(p, WW_DUEL_WEAPON, WW_DUEL_AMMO);
		SetPlayerArmedWeapon(p, WEAPON:WW_DUEL_WEAPON);
		TogglePlayerControllable(p, false);
		SendClientMessage(p, COLOR_WW, WW_TAG "Дуэль! Не двигайтесь до сигнала. Рука — на кобуре...");
	}

	new msg[160];
	format(msg, sizeof(msg), WW_TAG "%s и %s сходятся на дуэли. Ставка: $%d с каждого.",
		gPlayerData[a][pName], gPlayerData[b][pName], stake);
	WW_Broadcast(msg);
	return 1;
}

CMD:duel(playerid, params[])
{
	if (!WW_CanAct(playerid)) return 1;
	new targetid, stake;
	if (sscanf(params, "ud", targetid, stake)) {
		return SendClientMessage(playerid, COLOR_SYNTAX, "Использование: /duel [id/имя] [ставка 0-5000]");
	}
	if (targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid) || !gPlayerData[targetid][pLoggedIn]) {
		return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Игрок не найден.");
	}
	if (targetid == playerid) return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Стреляться с самим собой? Не дури.");
	if (stake < 0 || stake > WW_DUEL_MAX_STAKE) {
		return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Ставка от $0 до $5000.");
	}
	if (gWWDuelState[playerid] != WW_DUEL_NONE || gWWDuelState[targetid] != WW_DUEL_NONE) {
		return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Кто-то из вас уже на дуэли.");
	}
	if (gPlayerData[targetid][pInjured] || gPlayerData[targetid][pCuffed]) {
		return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Противник сейчас не в состоянии стреляться.");
	}
	new Float:tx, Float:ty, Float:tz;
	GetPlayerPos(targetid, tx, ty, tz);
	if (!IsPlayerInRangeOfPoint(playerid, WW_DUEL_RANGE, tx, ty, tz) || IsPlayerInAnyVehicle(playerid) || IsPlayerInAnyVehicle(targetid)) {
		return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Подойдите к противнику (до 20 м), оба должны стоять на ногах.");
	}
	if (gPlayerData[playerid][pMoney] < stake) {
		return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "У вас нет таких денег на ставку.");
	}

	gWWInviteFrom[targetid] = playerid;
	gWWInviteStake[targetid] = stake;
	gWWInviteTick[targetid] = gettime();

	SendRP(playerid, COLOR_WW, WW_TAG "Вы бросили вызов %s. Ставка: $%d. Ждём ответа...", gPlayerData[targetid][pName], stake);
	SendRP(targetid, COLOR_WW, WW_TAG "%s вызывает вас на дуэль! Ставка: $%d.", gPlayerData[playerid][pName], stake);
	SendClientMessage(targetid, COLOR_WW, WW_TAG "Принять: /duelaccept   Отказаться: /dueldecline   (30 секунд)");
	return 1;
}

CMD:duelaccept(playerid, params[])
{
	#pragma unused params
	if (!WW_CanAct(playerid)) return 1;
	new from = gWWInviteFrom[playerid];
	if (from == INVALID_PLAYER_ID || !IsPlayerConnected(from) || !gPlayerData[from][pLoggedIn]) {
		gWWInviteFrom[playerid] = INVALID_PLAYER_ID;
		return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Вас никто не вызывал на дуэль.");
	}
	new stake = gWWInviteStake[playerid];
	gWWInviteFrom[playerid] = INVALID_PLAYER_ID;

	if (gWWDuelState[playerid] != WW_DUEL_NONE || gWWDuelState[from] != WW_DUEL_NONE) {
		return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Кто-то из вас уже на дуэли.");
	}
	if (gPlayerData[from][pInjured] || gPlayerData[from][pCuffed]) {
		return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Противник сейчас не в состоянии стреляться.");
	}
	new Float:fx, Float:fy, Float:fz;
	GetPlayerPos(from, fx, fy, fz);
	if (!IsPlayerInRangeOfPoint(playerid, WW_DUEL_RANGE, fx, fy, fz) || IsPlayerInAnyVehicle(playerid) || IsPlayerInAnyVehicle(from)) {
		return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Противник слишком далеко или в транспорте.");
	}
	if (gPlayerData[playerid][pMoney] < stake || gPlayerData[from][pMoney] < stake) {
		return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "У кого-то из вас не хватает денег на ставку.");
	}
	if (!WW_StartDuel(from, playerid, stake)) {
		return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Не удалось начать дуэль.");
	}
	return 1;
}

CMD:dueldecline(playerid, params[])
{
	#pragma unused params
	new from = gWWInviteFrom[playerid];
	gWWInviteFrom[playerid] = INVALID_PLAYER_ID;
	if (from == INVALID_PLAYER_ID) return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Вас никто не вызывал на дуэль.");
	SendClientMessage(playerid, COLOR_WW, WW_TAG "Вы отказались от дуэли.");
	if (IsPlayerConnected(from)) {
		SendRP(from, COLOR_WW, WW_TAG "%s отказался от дуэли. Трус или мудрец?", gPlayerData[playerid][pName]);
	}
	return 1;
}

// ---------------------------------------------------------------------------
// Смерть: дуэли, награды, розыск. Вызывается из OnPlayerDeath.
// ---------------------------------------------------------------------------
stock WW_OnPlayerDeath(playerid, killerid)
{
	// 1. Дуэль
	if (gWWDuelState[playerid] != WW_DUEL_NONE) {
		new opp = gWWDuelOpponent[playerid];
		if (opp != INVALID_PLAYER_ID && IsPlayerConnected(opp)) {
			if (gWWDuelState[playerid] == WW_DUEL_COUNTDOWN) {
				// погиб до сигнала (упал, сбили) — ничья
				WW_DrawDuel(playerid, opp);
			} else {
				WW_FinishDuel(opp, playerid);
			}
		} else {
			WW_ClearDuel(playerid);
		}
		return 1; // дуэль — не преступление
	}

	if (killerid == INVALID_PLAYER_ID || killerid == playerid || !IsPlayerConnected(killerid)) return 0;
	if (!gWWLoaded[playerid] || !gWWLoaded[killerid]) return 0;

	// 2. Убит разыскиваемый — охотник получает награду
	new total = WW_GetTotalBounty(playerid);
	if (total > 0) {
		new reward = gWWBounty[playerid];
		// часть розыска шерифа взыскивается из кармана самого преступника
		new fine = gWWWanted[playerid];
		if (fine > gPlayerData[playerid][pMoney]) fine = gPlayerData[playerid][pMoney];
		if (fine > 0 && TakePlayerMoneyEx(playerid, fine, "взыскание по розыску")) {
			reward += fine;
		}
		gWWBounty[playerid] = 0;
		gWWWanted[playerid] = 0;

		if (reward > 0) GivePlayerMoneyEx(killerid, reward, "награда за голову");
		gWWClaimed[killerid]++;
		if (WW_GetTotalBounty(killerid) == 0) WW_AddHonor(killerid, 5);
		GivePlayerExp(killerid, 20, "награда за голову");

		new msg[160];
		format(msg, sizeof(msg), WW_TAG "Охотник %s взял голову %s и получил $%d.",
			gPlayerData[killerid][pName], gPlayerData[playerid][pName], reward);
		WW_Broadcast(msg);
		return 1;
	}

	// 3. Убит невиновный — убийца объявляется в розыск (кроме шерифа на службе)
	if (!WW_IsSheriff(killerid)) {
		gWWWanted[killerid] += WW_MURDER_WANTED;
		WW_AddHonor(killerid, -10);
		SendRP(killerid, COLOR_ERROR, WW_TAG "Вы убили невиновного. Розыск +$%d, честь падает.", WW_MURDER_WANTED);

		new msg[160];
		format(msg, sizeof(msg), "[ШЕРИФ] %s объявлен в розыск за убийство. Награда: $%d.",
			gPlayerData[killerid][pName], WW_GetTotalBounty(killerid));
		WW_AlertSheriffs(msg);
	}
	return 0;
}

// ---------------------------------------------------------------------------
// Награды за голову
// ---------------------------------------------------------------------------
CMD:bounty(playerid, params[])
{
	if (!WW_CanAct(playerid)) return 1;
	new targetid, amount;
	if (sscanf(params, "ud", targetid, amount)) {
		return SendClientMessage(playerid, COLOR_SYNTAX, "Использование: /bounty [id/имя] [сумма]  — назначить награду за голову");
	}
	if (targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid) || !gPlayerData[targetid][pLoggedIn] || !gWWLoaded[targetid]) {
		return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Игрок не найден.");
	}
	if (targetid == playerid) return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Назначить награду за себя? Смелое решение, но нет.");
	if (amount < WW_BOUNTY_MIN || amount > WW_BOUNTY_MAX) {
		return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Сумма награды: от $100 до $100000.");
	}
	if (!TakePlayerMoneyEx(playerid, amount, "награда за голову")) {
		return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "У вас нет таких денег.");
	}
	gWWBounty[targetid] += amount;

	new msg[160];
	format(msg, sizeof(msg), WW_TAG "За голову %s назначена награда $%d. Всего: $%d.",
		gPlayerData[targetid][pName], amount, WW_GetTotalBounty(targetid));
	WW_Broadcast(msg);
	return 1;
}

CMD:bounties(playerid, params[])
{
	#pragma unused params
	if (!gPlayerData[playerid][pLoggedIn]) return 1;

	new body[1536], line[96], count = 0;
	format(body, sizeof(body), "Имя\tНаграда\tИз них розыск\n");
	for (new i = 0; i < MAX_PLAYERS; i++) {
		if (!IsPlayerConnected(i) || !gPlayerData[i][pLoggedIn] || !gWWLoaded[i]) continue;
		new total = WW_GetTotalBounty(i);
		if (total <= 0) continue;
		format(line, sizeof(line), "%s (ID %d)\t{E74C3C}$%d\t{AFAFAF}$%d\n", gPlayerData[i][pName], i, total, gWWWanted[i]);
		if (strlen(body) + strlen(line) >= sizeof(body)) break;
		strcat(body, line);
		count++;
	}
	if (count == 0) {
		return SendClientMessage(playerid, COLOR_WW, WW_TAG "Доска «Разыскивается» пуста. На Фронтире тихо... пока.");
	}
	Dialog_Show(playerid, WWBountyBoard, DIALOG_STYLE_TABLIST_HEADERS, "{D2A066}РАЗЫСКИВАЕТСЯ — живым или мёртвым", body, "Закрыть", "");
	return 1;
}

Dialog:WWBountyBoard(playerid, response, listitem, inputtext[])
{
	#pragma unused playerid, response, listitem, inputtext
	return 1;
}

// Погасить розыск шерифа (назначенные игроками награды так не снимаются)
CMD:payfine(playerid, params[])
{
	#pragma unused params
	if (!WW_CanAct(playerid)) return 1;
	if (!IsPlayerInRangeOfPoint(playerid, 4.0, WW_SHERIFF_X, WW_SHERIFF_Y, WW_SHERIFF_Z)) {
		return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Штраф платят в офисе шерифа.");
	}
	new fine = gWWWanted[playerid];
	if (fine <= 0) return SendClientMessage(playerid, COLOR_WW, WW_TAG "Шериф вас не разыскивает.");
	if (!TakePlayerMoneyEx(playerid, fine, "погашение розыска")) {
		return SendRP(playerid, COLOR_ERROR, WW_TAG "Нужно $%d наличными.", fine);
	}
	gWWWanted[playerid] = 0;
	WW_AddHonor(playerid, 3);
	SendRP(playerid, COLOR_SUCCESS, WW_TAG "Вы заплатили штраф $%d. Шериф снял вас с розыска.", fine);
	if (gWWBounty[playerid] > 0) {
		SendRP(playerid, COLOR_WARNING, WW_TAG "Но награда от частных лиц ($%d) всё ещё действует.", gWWBounty[playerid]);
	}
	return 1;
}

// ---------------------------------------------------------------------------
// Честь и справка
// ---------------------------------------------------------------------------
CMD:honor(playerid, params[])
{
	#pragma unused params
	if (!gPlayerData[playerid][pLoggedIn]) return 1;
	new title[32], body[512];
	WW_GetHonorTitle(playerid, title);
	format(body, sizeof(body),
		"{FFFFFF}Честь: {D2A066}%d{FFFFFF} / 100  —  %s\n\n\
Награда за вашу голову: {E74C3C}$%d{FFFFFF} (розыск шерифа: $%d)\n\
Сдано голов: %d\n\
Шкуры: %d   Мясо: %d\n\n\
{AFAFAF}Честь растёт за победы на дуэлях, поимку преступников и уплату штрафов.\n\
Падает за убийства и грабежи. От чести зависят цены в фактории.",
		gWWHonor[playerid], title, WW_GetTotalBounty(playerid), gWWWanted[playerid],
		gWWClaimed[playerid], gWWPelts[playerid], gWWMeat[playerid]);
	Dialog_Show(playerid, WWInfo, DIALOG_STYLE_MSGBOX, "{D2A066}РЕПУТАЦИЯ НА ФРОНТИРЕ", body, "Закрыть", "");
	return 1;
}

CMD:ww(playerid, params[])
{
	#pragma unused params
	Dialog_Show(playerid, WWInfo, DIALOG_STYLE_MSGBOX, "{D2A066}ДИКИЙ ЗАПАД — СПРАВКА",
		"{D2A066}Дуэли{FFFFFF}\n\
/duel [id] [ставка] — вызвать на дуэль (до 20 м)\n\
/duelaccept, /dueldecline — ответить на вызов\n\n\
{D2A066}Закон и награды{FFFFFF}\n\
/bounty [id] [сумма] — назначить награду за голову\n\
/bounties — доска «Разыскивается»\n\
/payfine — погасить розыск у шерифа\n\
/honor — ваша честь и статистика\n\n\
{D2A066}Жизнь на Фронтире{FFFFFF}\n\
/horse — конюшня: взять, подозвать, отпустить лошадь\n\
/hunt — охота в угодьях (винтовка/дробовик в руках)\n\
/buyrifle, /sellpelts — фактория\n\
/deadeye — «Меткий глаз»: +50% урона на 10 сек\n\n\
{D2A066}Преступный мир{FFFFFF}\n\
/robcoach — ограбить дилижанс (когда появится на дороге)",
		"Понял", "");
	return 1;
}

CMD:zapad(playerid, params[])
{
	return cmd_ww(playerid, params);
}

Dialog:WWInfo(playerid, response, listitem, inputtext[])
{
	#pragma unused playerid, response, listitem, inputtext
	return 1;
}

// ---------------------------------------------------------------------------
// Лошадь
// ---------------------------------------------------------------------------
stock WW_DestroyHorse(playerid)
{
	if (gWWHorse[playerid] != INVALID_VEHICLE_ID) {
		if (IsValidVehicle(gWWHorse[playerid])) DestroyVehicle(gWWHorse[playerid]);
		gWWHorse[playerid] = INVALID_VEHICLE_ID;
	}
	return 1;
}

stock bool:WW_HorseAlive(playerid)
{
	return (gWWHorse[playerid] != INVALID_VEHICLE_ID && IsValidVehicle(gWWHorse[playerid]));
}

CMD:horse(playerid, params[])
{
	#pragma unused params
	if (!WW_CanAct(playerid)) return 1;
	Dialog_Show(playerid, WWHorseMenu, DIALOG_STYLE_LIST, "{D2A066}ЛОШАДЬ",
		"Взять лошадь в конюшне ($50)\nПодозвать свистом\nОтпустить лошадь",
		"Выбрать", "Закрыть");
	return 1;
}

Dialog:WWHorseMenu(playerid, response, listitem, inputtext[])
{
	#pragma unused inputtext
	if (!response || !WW_CanAct(playerid)) return 1;

	switch (listitem) {
		case 0: {
			if (!IsPlayerInRangeOfPoint(playerid, 6.0, WW_STABLE_X, WW_STABLE_Y, WW_STABLE_Z)) {
				return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Лошадь можно взять только в конюшне (метка на карте).");
			}
			if (WW_HorseAlive(playerid)) {
				return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "У вас уже есть лошадь. Подзовите её свистом.");
			}
			new price = WW_BuyPrice(playerid, WW_HORSE_PRICE);
			if (!TakePlayerMoneyEx(playerid, price, "лошадь из конюшни")) {
				return SendRP(playerid, COLOR_ERROR, WW_TAG "Конюх просит $%d.", price);
			}
			new v = CreateVehicle(WW_HORSE_MODEL, WW_HORSE_SPAWN_X, WW_HORSE_SPAWN_Y, WW_HORSE_SPAWN_Z, WW_HORSE_SPAWN_A, 0, 0, -1);
			if (v == INVALID_VEHICLE_ID) {
				GivePlayerMoneyEx(playerid, price, "возврат за лошадь");
				return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Свободных лошадей нет, попробуйте позже.");
			}
			SetVehicleParamsEx(v, 1, 0, 0, 0, 0, 0, 0);
			gWWHorse[playerid] = v;
			SendRP(playerid, COLOR_SUCCESS, WW_TAG "Конюх выводит вам лошадь за $%d. Берегите её!", price);
		}
		case 1: {
			if (!WW_HorseAlive(playerid)) {
				return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "У вас нет лошади. Загляните в конюшню.");
			}
			if (IsPlayerInAnyVehicle(playerid) || GetPlayerInterior(playerid) != 0) {
				return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Свистеть можно только на улице и пешком.");
			}
			new now = gettime();
			if (now - gWWHorseCallTick[playerid] < WW_HORSE_CALL_COOLDOWN) {
				return SendRP(playerid, COLOR_ERROR, WW_TAG "Лошадь уже бежит. Подождите %d сек.",
					WW_HORSE_CALL_COOLDOWN - (now - gWWHorseCallTick[playerid]));
			}
			if (WW_IsVehicleOccupied(gWWHorse[playerid])) {
				return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Кто-то сидит в седле вашей лошади! Конокрад?");
			}
			new Float:x, Float:y, Float:z, Float:a;
			GetPlayerPos(playerid, x, y, z);
			GetPlayerFacingAngle(playerid, a);
			// лошадь подбегает сбоку, в 4 метрах
			x += 4.0 * floatsin(-(a + 90.0), degrees);
			y += 4.0 * floatcos(-(a + 90.0), degrees);
			SetVehiclePos(gWWHorse[playerid], x, y, z + 0.5);
			SetVehicleZAngle(gWWHorse[playerid], a);
			SetVehicleVirtualWorld(gWWHorse[playerid], GetPlayerVirtualWorld(playerid));
			SetVehicleParamsEx(gWWHorse[playerid], 1, 0, 0, 0, 0, 0, 0);
			gWWHorseCallTick[playerid] = now;
			SendRP(playerid, COLOR_ME, "* %s свистит, подзывая лошадь.", gPlayerData[playerid][pName]);
		}
		case 2: {
			if (!WW_HorseAlive(playerid)) {
				return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "У вас нет лошади.");
			}
			if (WW_IsVehicleOccupied(gWWHorse[playerid])) {
				return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Сначала все должны спешиться.");
			}
			WW_DestroyHorse(playerid);
			SendClientMessage(playerid, COLOR_WW, WW_TAG "Вы отпустили лошадь — она сама найдёт дорогу в конюшню.");
		}
	}
	return 1;
}

// ---------------------------------------------------------------------------
// Охота и фактория
// ---------------------------------------------------------------------------
CMD:hunt(playerid, params[])
{
	#pragma unused params
	if (!WW_CanAct(playerid)) return 1;
	if (!IsPlayerInRangeOfPoint(playerid, WW_HUNT_RANGE, WW_HUNT_X, WW_HUNT_Y, WW_HUNT_Z)) {
		return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Здесь нет дичи. Отправляйтесь в охотничьи угодья (метка на карте).");
	}
	if (IsPlayerInAnyVehicle(playerid)) {
		return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Со зверем на колёсах не подружишься. Спешьтесь.");
	}
	if (!WW_HasHuntingWeapon(playerid)) {
		return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Возьмите в руки винтовку или дробовик (/buyrifle в фактории).");
	}
	new now = gettime();
	if (now < gWWHuntReady[playerid]) {
		return SendRP(playerid, COLOR_ERROR, WW_TAG "Вы спугнули округу. Подождите %d сек.", gWWHuntReady[playerid] - now);
	}
	gWWHuntReady[playerid] = now + WW_HUNT_COOLDOWN;
	ApplyAnimation(playerid, "BOMBER", "BOM_Plant", 4.1, true, false, false, false, 0);
	ApplyProgressBar(playerid, "ВЫСЛЕЖИВАНИЕ ЗВЕРЯ...", WW_HUNT_TIME, "OnWWHuntTracked", 0);
	SendRP(playerid, COLOR_ME, "* %s присаживается и изучает следы на земле.", gPlayerData[playerid][pName]);
	return 1;
}

public OnWWHuntTracked(playerid, extra)
{
	#pragma unused extra
	if (!IsPlayerConnected(playerid) || !gPlayerData[playerid][pLoggedIn]) return 0;
	ClearAnimations(playerid);
	TogglePlayerControllable(playerid, true);
	if (gPlayerData[playerid][pInjured] || gPlayerData[playerid][pCuffed]) return 0;

	gWWHuntTrack[playerid] = random(3);
	Dialog_Show(playerid, WWHuntChoice, DIALOG_STYLE_LIST, "{2ECC71}ОХОТА — КУДА ВЕДУТ СЛЕДЫ?",
		"Вдоль ручья, к водопою\nВ густой кустарник на склоне\nК поваленным деревьям на холме",
		"Идти", "Бросить");
	return 1;
}

Dialog:WWHuntChoice(playerid, response, listitem, inputtext[])
{
	#pragma unused inputtext
	if (!response) return SendClientMessage(playerid, COLOR_GREY, WW_TAG "Вы бросили след.");
	if (!WW_CanAct(playerid)) return 1;
	if (!IsPlayerInRangeOfPoint(playerid, WW_HUNT_RANGE, WW_HUNT_X, WW_HUNT_Y, WW_HUNT_Z)) return 1;
	if (!WW_HasHuntingWeapon(playerid)) {
		return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Зверь рядом, а оружия в руках нет. Добыча ушла.");
	}

	if (listitem != gWWHuntTrack[playerid]) {
		SendClientMessage(playerid, COLOR_GREY, WW_TAG "Следы оборвались. Зверь ушёл — повезёт в другой раз.");
		GivePlayerExp(playerid, 2, "охота");
		return 1;
	}

	new roll = random(100), pelts, meat, animal[24];
	if (roll < 50)      { pelts = 1; meat = 1; format(animal, sizeof(animal), "зайца"); }
	else if (roll < 85) { pelts = 2; meat = 2; format(animal, sizeof(animal), "оленя"); }
	else                { pelts = 3; meat = 3; format(animal, sizeof(animal), "медведя"); }

	gWWPelts[playerid] += pelts;
	gWWMeat[playerid] += meat;
	GivePlayerExp(playerid, 6 + pelts * 3, "охота");
	PlayerPlaySound(playerid, 1057, 0.0, 0.0, 0.0);
	SendRP(playerid, COLOR_SUCCESS, WW_TAG "Метко! Вы добыли %s: шкуры +%d, мясо +%d.", animal, pelts, meat);
	SendRP(playerid, COLOR_ME, "* %s вскидывает ружьё и делает точный выстрел.", gPlayerData[playerid][pName]);

	// медведь может зацепить охотника
	if (pelts == 3 && random(100) < 30) {
		new Float:hp;
		GetPlayerHealth(playerid, hp);
		hp -= 30.0;
		if (hp < 5.0) hp = 5.0;
		SetPlayerHealth(playerid, hp);
		SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Раненый медведь успел полоснуть вас когтями! (-30 HP)");
	}
	return 1;
}

CMD:buyrifle(playerid, params[])
{
	#pragma unused params
	if (!WW_CanAct(playerid)) return 1;
	if (!IsPlayerInRangeOfPoint(playerid, 4.0, WW_TRADER_X, WW_TRADER_Y, WW_TRADER_Z)) {
		return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Винтовки продают в фактории (метка на карте).");
	}
	new price = WW_BuyPrice(playerid, WW_RIFLE_PRICE);
	if (!TakePlayerMoneyEx(playerid, price, "охотничья винтовка")) {
		return SendRP(playerid, COLOR_ERROR, WW_TAG "Торговец просит $%d.", price);
	}
	AC_GiveWeapon(playerid, _:WEAPON_RIFLE, WW_RIFLE_AMMO);
	SendRP(playerid, COLOR_SUCCESS, WW_TAG "Вы купили охотничью винтовку с %d патронами за $%d.", WW_RIFLE_AMMO, price);
	PlayerPlaySound(playerid, 1052, 0.0, 0.0, 0.0);
	return 1;
}

CMD:sellpelts(playerid, params[])
{
	#pragma unused params
	if (!WW_CanAct(playerid)) return 1;
	if (!IsPlayerInRangeOfPoint(playerid, 4.0, WW_TRADER_X, WW_TRADER_Y, WW_TRADER_Z)) {
		return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Шкуры принимают в фактории (метка на карте).");
	}
	new pelts = gWWPelts[playerid], meat = gWWMeat[playerid];
	if (pelts <= 0 && meat <= 0) return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Вам нечего продать.");

	new total = pelts * WW_SellPrice(playerid, WW_PELT_PRICE) + meat * WW_SellPrice(playerid, WW_MEAT_PRICE);
	gWWPelts[playerid] = 0;
	gWWMeat[playerid] = 0;
	GivePlayerMoneyEx(playerid, total, "продажа шкур и мяса");
	SendRP(playerid, COLOR_SUCCESS, WW_TAG "Торговец забрал %d шкур и %d мяса. Выручка: $%d.", pelts, meat, total);
	PlayerPlaySound(playerid, 1052, 0.0, 0.0, 0.0);
	WW_SavePlayer(playerid);
	return 1;
}

// ---------------------------------------------------------------------------
// Меткий глаз
// ---------------------------------------------------------------------------
CMD:deadeye(playerid, params[])
{
	#pragma unused params
	if (!WW_CanAct(playerid)) return 1;
	new now = gettime();
	if (gWWDeadEyeEnd[playerid] > now) return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "«Меткий глаз» уже активен.");
	if (now < gWWDeadEyeReady[playerid]) {
		return SendRP(playerid, COLOR_ERROR, WW_TAG "Глаза ещё не отдохнули. Ждите %d сек.", gWWDeadEyeReady[playerid] - now);
	}
	gWWDeadEyeEnd[playerid] = now + WW_DEADEYE_TIME;
	gWWDeadEyeReady[playerid] = now + WW_DEADEYE_COOLDOWN;
	GameTextForPlayer(playerid, "~r~DEAD EYE", 2000, 3);
	PlayerPlaySound(playerid, 1137, 0.0, 0.0, 0.0);
	SendClientMessage(playerid, COLOR_WW, WW_TAG "Время будто замедлилось... Ваши выстрелы на 50% смертоноснее (10 сек).");
	return 1;
}

// Бонусный урон «Меткого глаза». Вызывается из OnPlayerGiveDamage (после античита).
stock WW_OnPlayerGiveDamage(playerid, damagedid, Float:amount, WEAPON:weaponid)
{
	if (gWWDeadEyeEnd[playerid] == 0 || gettime() >= gWWDeadEyeEnd[playerid]) return 0;
	if (damagedid == INVALID_PLAYER_ID || !IsPlayerConnected(damagedid)) return 0;
	if (_:weaponid < 22 || _:weaponid > 34) return 0; // только огнестрел
	if (gPlayerData[damagedid][pInjured]) return 0;

	new Float:hp;
	GetPlayerHealth(damagedid, hp);
	hp -= amount * 0.5;
	if (hp < 0.0) hp = 0.0;
	SetPlayerHealth(damagedid, hp);
	return 1;
}

// ---------------------------------------------------------------------------
// Дилижанс
// ---------------------------------------------------------------------------
stock WW_DespawnCoach()
{
	if (gWWCoachLabel != INVALID_3DTEXT_ID) {
		Delete3DTextLabel(gWWCoachLabel);
		gWWCoachLabel = INVALID_3DTEXT_ID;
	}
	if (gWWCoachVeh != INVALID_VEHICLE_ID) {
		if (IsValidVehicle(gWWCoachVeh)) DestroyVehicle(gWWCoachVeh);
		gWWCoachVeh = INVALID_VEHICLE_ID;
	}
	gWWCoachRobber = INVALID_PLAYER_ID;
	gWWCoachNext = gettime() + WW_COACH_INTERVAL;
	return 1;
}

stock WW_SpawnCoach()
{
	if (gWWCoachVeh != INVALID_VEHICLE_ID) return 0;
	new s = random(sizeof(gWWCoachSpots));
	gWWCoachVeh = CreateVehicle(WW_COACH_MODEL, gWWCoachSpots[s][0], gWWCoachSpots[s][1], gWWCoachSpots[s][2], gWWCoachSpots[s][3], 1, 1, -1);
	if (gWWCoachVeh == INVALID_VEHICLE_ID) {
		gWWCoachNext = gettime() + 60;
		return 0;
	}
	// двери заперты, двигатель заглушен: угнать нельзя, только ограбить
	SetVehicleParamsEx(gWWCoachVeh, 0, 0, 0, 1, 0, 0, 1);
	gWWCoachLabel = Create3DTextLabel("{D2A066}[ ДИЛИЖАНС «ЮНИОН ЭКСПРЕСС» ]\n{FFFFFF}Везёт золото банку\n{E74C3C}/robcoach{FFFFFF} — ограбить",
		COLOR_WW, 0.0, 0.0, 0.0, 25.0, 0, true);
	Attach3DTextLabelToVehicle(gWWCoachLabel, gWWCoachVeh, 0.0, 0.0, 1.5);

	gWWCoachExpire = gettime() + WW_COACH_LIFETIME;
	gWWCoachRobber = INVALID_PLAYER_ID;
	WW_Broadcast(WW_TAG "Ходят слухи: дилижанс с золотом остановился у Форт-Карсона. Шериф, будьте начеку!");
	return 1;
}

stock WW_CoachTick(now)
{
	if (gWWCoachVeh == INVALID_VEHICLE_ID) {
		if (now >= gWWCoachNext) {
			if (WW_CountOnline() >= WW_COACH_MIN_PLAYERS) WW_SpawnCoach();
			else gWWCoachNext = now + 120;
		}
		return 1;
	}
	if (!IsValidVehicle(gWWCoachVeh)) {
		gWWCoachVeh = INVALID_VEHICLE_ID;
		WW_DespawnCoach();
		return 1;
	}
	// грабитель пропал / упал / бросил — дилижанс снова свободен
	if (gWWCoachRobber != INVALID_PLAYER_ID && now - gWWCoachRobTick > WW_COACH_ROB_TIME + 5) {
		gWWCoachRobber = INVALID_PLAYER_ID;
	}
	if (now >= gWWCoachExpire && gWWCoachRobber == INVALID_PLAYER_ID) {
		WW_Broadcast(WW_TAG "Дилижанс благополучно уехал. Золото доставлено в банк.");
		WW_DespawnCoach();
	}
	return 1;
}

CMD:robcoach(playerid, params[])
{
	#pragma unused params
	if (!WW_CanAct(playerid)) return 1;
	if (gWWCoachVeh == INVALID_VEHICLE_ID) return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Дилижанса поблизости нет.");
	if (WW_IsSheriff(playerid)) return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Шериф на службе грабить не может. Снимите звезду (/duty).");
	if (IsPlayerInAnyVehicle(playerid)) return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Спешьтесь.");

	new Float:x, Float:y, Float:z;
	GetVehiclePos(gWWCoachVeh, x, y, z);
	if (!IsPlayerInRangeOfPoint(playerid, 6.0, x, y, z)) {
		return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Подойдите вплотную к дилижансу.");
	}
	if (gWWCoachRobber != INVALID_PLAYER_ID && gWWCoachRobber != playerid) {
		return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Дилижанс уже кто-то грабит!");
	}
	gWWCoachRobber = playerid;
	gWWCoachRobTick = gettime();

	ApplyAnimation(playerid, "BOMBER", "BOM_Plant", 4.1, true, false, false, false, 0);
	ApplyProgressBar(playerid, "ВЗЛОМ СУНДУКА С ЗОЛОТОМ...", WW_COACH_ROB_TIME, "OnWWCoachRobbed", 0);
	SendRP(playerid, COLOR_ME, "* %s взламывает сундук дилижанса.", gPlayerData[playerid][pName]);

	new msg[144];
	format(msg, sizeof(msg), "[ШЕРИФ] Ограбление дилижанса! Подозреваемый: %s. Координаты отмечены на карте.", gPlayerData[playerid][pName]);
	WW_AlertSheriffs(msg);
	for (new i = 0; i < MAX_PLAYERS; i++) {
		if (IsPlayerConnected(i) && gPlayerData[i][pLoggedIn] && gPlayerData[i][pFaction] == FACTION_POLICE) {
			SetPlayerCheckpoint(i, x, y, z, 5.0);
		}
	}
	return 1;
}

public OnWWCoachRobbed(playerid, extra)
{
	#pragma unused extra
	if (!IsPlayerConnected(playerid) || !gPlayerData[playerid][pLoggedIn]) return 0;
	ClearAnimations(playerid);
	TogglePlayerControllable(playerid, true);
	if (gPlayerData[playerid][pInjured] || gPlayerData[playerid][pCuffed]) return 0;
	if (gWWCoachRobber != playerid || gWWCoachVeh == INVALID_VEHICLE_ID || !IsValidVehicle(gWWCoachVeh)) return 0;

	new Float:x, Float:y, Float:z;
	GetVehiclePos(gWWCoachVeh, x, y, z);
	if (!IsPlayerInRangeOfPoint(playerid, 8.0, x, y, z)) {
		gWWCoachRobber = INVALID_PLAYER_ID;
		return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Вы отошли от дилижанса — взлом сорван.");
	}

	new loot = WW_COACH_REWARD_MIN + random(WW_COACH_REWARD_RAND);
	GivePlayerMoneyEx(playerid, loot, "ограбление дилижанса");
	gWWWanted[playerid] += WW_COACH_WANTED;
	WW_AddHonor(playerid, -15);
	GivePlayerExp(playerid, 25, "ограбление дилижанса");
	PlayerPlaySound(playerid, 1058, 0.0, 0.0, 0.0);
	SendRP(playerid, COLOR_SUCCESS, WW_TAG "Сундук вскрыт! Добыча: $%d. Уносите ноги — вас ищет шериф.", loot);

	new msg[160];
	format(msg, sizeof(msg), WW_TAG "Дилижанс ограблен! %s в розыске. Награда за голову: $%d.",
		gPlayerData[playerid][pName], WW_GetTotalBounty(playerid));
	WW_Broadcast(msg);

	WW_DespawnCoach();
	return 1;
}

// Админ: вызвать дилижанс немедленно
CMD:wwcoach(playerid, params[])
{
	#pragma unused params
	if (gPlayerData[playerid][pAdmin] < 3) return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Недостаточно прав.");
	if (gWWCoachVeh != INVALID_VEHICLE_ID) return SendClientMessage(playerid, COLOR_ERROR, WW_TAG "Дилижанс уже на дороге.");
	WW_SpawnCoach();
	return 1;
}
