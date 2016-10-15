/*
* Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
* Copyright (C) 2016-20XX JadeCore <https://www.jadecore.tk/>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "heart_of_fear.h"

enum Achievements
{
    ACHIEVEMENT_RETURN_TO_SENDER = 6485,
};

enum Spells
{

};

enum Mobs
{
    // Heroic Adds
    NPC_ECHO_OF_ATTENUATION     = 65173,
    NPC_ECHO_OF_FORCE_AND_VERVE = 65174,
};

enum Events
{
	EVENT_TALK_INTRO_TWO     = 1,
	EVENT_TALK_INTRO_THREE   = 2,
	EVENT_TALK_INTRO_FOUR    = 3,
	EVENT_TALK_INTRO_FIVE    = 4,
	EVENT_TALK_INTRO_SIX     = 5,
};

enum Objects
{
    GOB_ARENA_WALLS       = 212916,
    GOB_FINAL_PHASE_WALLS = 212943,
};

enum Action
{
	ACTION_PRE_FIGHT_EVENT    = 1,
	ACTION_TALK_TRASH_A_START = 2,
	ACTION_TALK_TRASH_A_DIE   = 3,
	ACTION_TALK_TRASH_B_START = 4,
	ACTION_TALK_TRASH_B_DIE   = 5,
	ACTION_TALK_TRASH_C_START = 6,
	ACTION_TALK_TRASH_C_DIE   = 7,
};

// Talk is done in boss script not in database
enum Talk { };

Position const Ramp_Pos1 = { -2236.312744f, 217.689651f, 2.556486f };
Position const Ramp_Pos2 = { -2317.847900f, 299.153625f, 409.896881f };
Position const Ramp_Pos3 = { -2315.115967f, 218.375854f, 409.897125f };


Position finalPhaseWalls1[3] =
{
    { -2299.195f, 282.5938f, 408.5445f, 2.383867f },
    { -2250.401f, 234.0122f, 408.5445f, 2.333440f },
    { -2299.63f, 233.3889f, 408.5445f, 0.7598741f }
};

Position finalPhaseWalls2[3] =
{
    { -2255.168f, 308.7326f, 406.0f, 0.7853968f },
    { -2240.0f, 294.0f, 406.0f, 0.7853968f },
    { -2225.753f, 280.1424f, 406.381f, 0.7853968f },
};

static void ScreenText(Creature* creature, const char *text)
{
	if (creature->GetGUID() == NULL)
		return;

	creature->MonsterTextEmote(text, creature->GetGUID(), true);
}

void PlaySound(WorldObject* source, uint32 soundId)
{
	if (!source)
		return;

	source->PlayDirectSound(soundId);
}

static void SoundYell(Creature* creature, const char *text, uint32 soundId)
{
	if (creature->GetGUID() == NULL)
		return;

	creature->MonsterYell(text, LANG_UNIVERSAL, creature->GetGUID());
	PlaySound(creature, soundId);
}

class boss_imperial_zorlok : public CreatureScript
{
public:
    boss_imperial_zorlok() : CreatureScript("boss_imperial_zorlok") { }

    struct boss_imperial_zorlokAI : public BossAI
    {
        boss_imperial_zorlokAI(Creature* creature) : BossAI(creature, DATA_IMPERIAL_VIZIER_ZORLOK)
        {
            me->SetLevel(93);
            me->setFaction(16);
            me->SetSpeed(MOVE_RUN, 3.5f, true);
            me->SetSpeed(MOVE_FLIGHT, 3.5f, true);
			me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        }

        InstanceScript* pInstance;
		uint8 trashMobsKilled = 0;

        void Reset()
        {
            _Reset();

			switch (me->GetMap()->GetDifficulty())
			{
				case MAN10_DIFFICULTY:
					me->SetMaxHealth(174454800);
					me->SetFullHealth();
					break;
				case MAN10_HEROIC_DIFFICULTY:
					me->SetMaxHealth(218067496);
					me->SetFullHealth();
					break;
				case MAN25_DIFFICULTY:
					me->SetMaxHealth(392523296);
					me->SetFullHealth();
					break;
				case MAN25_HEROIC_DIFFICULTY:
					me->SetMaxHealth(588784960);
					me->SetFullHealth();
					break;
			}
        }

        void EnterCombat(Unit* who)
        {
            _EnterCombat();

            // Set new home position
            me->SetHomePosition(-2291.480957f, 243.480286f, 422.678986f, 0.753832f);
        }

        void JustReachedHome()
        {
            summons.DespawnAll();
        }

        void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/)
        {

        }

		void JustDied(Unit* /*killer*/)
        {
            _JustDied();
			SoundYell(me, "We will not give in to the despair of the dark void. If Her will for us is to perish, then it shall be so.", 29302);
        }

        void KilledUnit(Unit* /*victim*/)
        {

        }

        void RampChange(bool message)
        {
            MotionMaster* motion = me->GetMotionMaster();

            if (message)
            {
				ScreenText(me, "Imperial Vizier Zor'lok flies to one of his platforms!");

                if (motion)
                    motion->MovePoint(1, me->GetPositionX(), me->GetPositionY(), 423.399048f);

                me->SetReactState(REACT_PASSIVE);
                me->AddUnitState(UNIT_STATE_CANNOT_AUTOATTACK);
                message = false;
            }
        }

        void CenterChange()
        {
            if (MotionMaster* motion = me->GetMotionMaster())
            {
                motion->MovePoint(1, -2274.567383f, 259.058289f, 420.271484f);
            }
        }

		void DoAction(const int32 action)
		{
			switch (action)
			{
				case ACTION_TALK_TRASH_A_START:
				{
					// It will start when trash A enters combat
					SoundYell(me, "The chaff of the world tumbles across our doorstep, driven by fear; Her royal swarm will whisk them away.", 29312);
					break;
				}

				case ACTION_TALK_TRASH_A_DIE:
				{
					// It will start when trash A dies
					SoundYell(me, "They were clearly unworthy of Her divine embrace.", 29313);
					break;
				}

				case ACTION_TALK_TRASH_B_START:
				{
					// It will start when trash B enters combat
					SoundYell(me, "They are but the waves crashing upon the mountain of Her divine will. They may rise again and again; but will accomplish nothing.", 29314);
					break;
				}

				case ACTION_TALK_TRASH_B_DIE:
				{
					// It will start when trash B dies
					SoundYell(me, "We are unfazed. We will stand firm.", 29315);
					break;
				}

				case ACTION_TALK_TRASH_C_START:
				{
					// It will start when trash C enters combat
					SoundYell(me, "The Divine challenges us to face these intruders.", 29316);
					break;
				}

				case ACTION_TALK_TRASH_C_DIE:
				{
					// It will start when trash C dies
					SoundYell(me, "And so it falls to us, Her chosen voice.", 29317);
					break;
				}

				case ACTION_PRE_FIGHT_EVENT:
				{
					trashMobsKilled++;
					if (trashMobsKilled == 100) // Need to find correct number...
					{
						SoundYell(me, "We are the extension of our Empress's will.", 29303);
						events.ScheduleEvent(EVENT_TALK_INTRO_TWO, 5000);
					}

					break;
				}
			}
		}

        void UpdateAI(uint32 const diff)
        {
            events.Update(diff);

            if (!UpdateVictim())
                return;

			if (me->HasUnitState(UNIT_STATE_CASTING))
				return;

			while (uint32 eventId = events.ExecuteEvent())
			{
				switch (eventId)
				{
					case EVENT_TALK_INTRO_TWO:
					{
						SoundYell(me, "Ours is but to serve in Her divine name.", 29304);
						events.ScheduleEvent(EVENT_TALK_INTRO_THREE, 5000);
						break;
					}

					case EVENT_TALK_INTRO_THREE:
					{
						SoundYell(me, "Never to question, nor to contemplate, we simply act.", 29305);
						events.ScheduleEvent(EVENT_TALK_INTRO_FOUR, 5000);
						break;
					}

					case EVENT_TALK_INTRO_FOUR:
					{
						SoundYell(me, "We fight, toil and serve so that Her vision is made for us reality.", 29306);
						events.ScheduleEvent(EVENT_TALK_INTRO_FIVE, 6000);
						break;
					}

					case EVENT_TALK_INTRO_FIVE:
					{
						SoundYell(me, "Her happiness is our reward, Her sorrow our failure.", 29307);
						events.ScheduleEvent(EVENT_TALK_INTRO_SIX, 9000);
						break;
					}

					case EVENT_TALK_INTRO_SIX:
					{
						SoundYell(me, "We will give our lives for the Empress without hesitation. She's our light and without Her our lives will be lost to darkness.", 29308);
						me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
						me->SetReactState(ReactStates::REACT_PASSIVE);
						break;
					}
				}
			}

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_imperial_zorlokAI(creature);
    }
};

void AddSC_imperial_zorlok()
{
    // Boss
    new boss_imperial_zorlok();
}