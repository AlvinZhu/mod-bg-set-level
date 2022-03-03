/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "UnitFields.h"

// Add player scripts
class PS_BGLevel : public PlayerScript
{
private:
    void RestoreOriginalLevel(Player* player)
    {
        ObjectGuid playerGUID = player->GetGUID();
        sT->entryMap.erase(playerGUID);
        QueryResult result = CharacterDatabase.Query("SELECT PlayerID, original_level, original_xp FROM custom_bg_level WHERE PlayerID = {}", player->GetGUID().GetCounter());
        if (result)
        {
            CharacterDatabase.Execute("DELETE FROM custom_bg_level WHERE PlayerID = {}", player->GetGUID().GetCounter());
            player->SetLevel((*result)[1].Get<uint8>(), false);
            player->SetUInt32Value(PLAYER_XP, (*result)[2].Get<uint32>());
        }
    }

    void SetToBGLevel(Player* player)
    {
        uint32 levelToSet = sConfigMgr->GetOption<uint32>("CustomBGLevel.Level", 80);
        CharacterDatabase.Execute("INSERT INTO custom_bg_level (PlayerID, original_level, original_xp) VALUES ({}, {}, {})", player->GetGUID().GetCounter(), player->getLevel(), player->GetUInt32Value(PLAYER_XP));
        player->SetLevel(levelToSet, false);
    }

public:
    PS_BGLevel() : PlayerScript("Player_BGLevel") { }

    void OnLogin(Player* player) override
    {
        if (sConfigMgr->GetOption<bool>("CustomBGLevel.Enable", false))
        {
            ChatHandler(player->GetSession()).SendSysMessage("Custom BG Level is Enabled");
            if (!player->InBattleground())
            {
                RestoreOriginalLevel(player);
            }
        }
    }

    void OnAddToBattleground(Player* player, Battleground* /*bg*/) override
    {
        if (sConfigMgr->GetOption<bool>("CustomBGLevel.Enable", false))
        {
            SetToBGLevel(player);
        }
    }

    void OnRemoveFromBattleground(Player* player, Battleground* /*bg*/) override
    {
        if (sConfigMgr->GetOption<bool>("CustomBGLevel.Enable", false))
        {
            RestoreOriginalLevel(player);
        }
    }
};

// Add all scripts in one
void AddSC_BGLevel()
{
    new PS_BGLevel();
}