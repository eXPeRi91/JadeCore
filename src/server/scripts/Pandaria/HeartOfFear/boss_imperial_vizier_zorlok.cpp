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
	// Talks
	EVENT_TALK_INTRO_TWO   = 1,
	EVENT_TALK_INTRO_THREE = 2,
	EVENT_TALK_INTRO_FOUR  = 3,
	EVENT_TALK_INTRO_FIVE  = 4,
	EVENT_TALK_INTRO_SIX   = 5,

	// Spells
	EVENT_INHALE           = 6,
	EVENT_EXHALE           = 7,
	EVENT_FORCE_AND_WEARVE = 8,
	EVENT_ATTENUATION      = 9,
	EVENT_CONVERT          = 10,

	// Phase events
	EVENT_PHASE_FIRST_PLAT,
	EVENT_PHASE_SECON_PLAT,
	EVENT_PHASE_THIRD_PLAT,
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

enum Phases
{
	PHASE_FIRST_PLATFORM  = 1,
	PHASE_SECOND_PLATFORM = 2,
	PHASE_THIRD_PLATFORM  = 3,
	PHASE_TWO             = 4,
};

// Talk is done in boss script not in database
enum Talk
{
	TALK_TRASH_A_START = 0,  // "The chaff of the world tumbles across our doorstep, driven by fear; Her royal swarm will whisk them away.", 29312
	TALK_TRASH_A_DIE   = 1,  // "They were clearly unworthy of Her divine embrace.", 29313
	TALK_TRASH_B_START = 2,  // "They are but the waves crashing upon the mountain of Her divine will. They may rise again and again; but will accomplish nothing.", 29314
	TALK_TRASH_B_DIE   = 3,  // "We are unfazed. We will stand firm.", 29315
	TALK_TRASH_C_START = 4,  // "The Divine challenges us to face these intruders.", 29316
	TALK_TRASH_C_DIE   = 5,  // "And so it falls to us, Her chosen voice.", 29317

	TALK_INTRO_ONE     = 6,  // "We are the extension of our Empress's will.", 29303
	TALK_INTRO_TWO     = 7,  // "Ours is but to serve in Her divine name.", 29304
	TALK_INTRO_THREE   = 8,  // "Never to question, nor to contemplate, we simply act.", 29305
	TALK_INTRO_FOUR    = 9,  // "We fight, toil and serve so that Her vision is made for us reality.", 29306
	TALK_INTRO_FIVE    = 10, // "Her happiness is our reward, Her sorrow our failure.", 29307
	TALK_INTRO_SIX     = 11, // "We will give our lives for the Empress without hesitation. She's our light and without Her our lives will be lost to darkness.", 29308

	TALK_AGGRO         = 12, // "The divine chose us to give mortal voice to Her divine will. We are but the vessel that enacts Her will.", 29301
	TALK_DEATH         = 13, // "We will not give in to the despair of the dark void. If Her will for us is to perish, then it shall be so.", 29302
};

Position const Ramp_Pos1 = { -2236.312744f, 217.689651f, 2.55648600f };
Position const Ramp_Pos2 = { -2317.847900f, 299.153625f, 409.896881f };
Position const Ramp_Pos3 = { -2315.115967f, 218.375854f, 409.897125f };


Position finalPhaseWalls1[3] =
{
    { -2299.195f, 282.5938f, 408.5445f, 2.3838670f },
    { -2250.401f, 234.0122f, 408.5445f, 2.3334400f },
    { -2299.630f, 233.3889f, 408.5445f, 0.7598741f },
};

Position finalPhaseWalls2[3] =
{
    { -2255.168f, 308.7326f, 406.000f, 0.7853968f },
    { -2240.000f, 294.0000f, 406.000f, 0.7853968f },
    { -2225.753f, 280.1424f, 406.381f, 0.7853968f },
};

static void ScreenText(Creature* creature, const char *text)
{
	if (creature->GetGUID() == NULL)
		return;

	creature->MonsterTextEmote(text, creature->GetGUID(), true);
}

static void PlaySound(WorldObject* source, uint32 soundId)
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
		bool firstTrashDead = false;
		bool secondTrashDead = false;
		bool thirdTrashDead = false;
		uint32 currentPhase = 0;
        
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

			events.SetPhase(PHASE_FIRST_PLATFORM);
			events.ScheduleEvent(EVENT_PHASE_FIRST_PLAT, 5000, 0, PHASE_FIRST_PLATFORM);
			RampChange(true);
			FirstPlatform(true);
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
			Talk(TALK_DEATH);
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

		void FirstPlatform(bool check)
		{
			if (check = true)
				if (MotionMaster* move = me->GetMotionMaster())
				{
					move->MovePoint(1, Ramp_Pos1);
					check = false;
				}
		}

		void SecondPlatform(bool check)
		{
			if (check = true)
				if (MotionMaster* move = me->GetMotionMaster())
				{
					move->MovePoint(1, Ramp_Pos2);
					check = false;
				}
		}

		void ThirdPlatform(bool check)
		{
			if (check = true)
				if (MotionMaster* move = me->GetMotionMaster())
				{
					move->MovePoint(1, Ramp_Pos3);
					check = false;
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
					Talk(TALK_TRASH_A_START);
					firstTrashDead = false;
					break;
				}

				case ACTION_TALK_TRASH_A_DIE:
				{
					// It will start when trash A dies
					Talk(TALK_TRASH_A_DIE);
					firstTrashDead = true;
					break;
				}

				case ACTION_TALK_TRASH_B_START:
				{
					// It will start when trash B enters combat
					Talk(TALK_TRASH_B_START);
					secondTrashDead = false;
					break;
				}

				case ACTION_TALK_TRASH_B_DIE:
				{
					// It will start when trash B dies
					Talk(TALK_TRASH_B_DIE);
					secondTrashDead = true;
					break;
				}

				case ACTION_TALK_TRASH_C_START:
				{
					// It will start when trash C enters combat
					Talk(TALK_TRASH_C_START);
					thirdTrashDead = false;
					break;
				}

				case ACTION_TALK_TRASH_C_DIE:
				{
					// It will start when trash C dies
					Talk(TALK_TRASH_C_DIE);
					thirdTrashDead = true;
					break;
				}

				case ACTION_PRE_FIGHT_EVENT:
				{
					if (firstTrashDead == true && secondTrashDead == true && thirdTrashDead == true)
					{
						Talk(TALK_INTRO_ONE);
						events.ScheduleEvent(EVENT_TALK_INTRO_TWO, 5000);
					}
					else
						return;

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

			if (events.IsInPhase(PHASE_FIRST_PLATFORM))
				currentPhase = PHASE_FIRST_PLATFORM;

			if (events.IsInPhase(PHASE_SECOND_PLATFORM))
				currentPhase = PHASE_SECOND_PLATFORM;

			if (events.IsInPhase(PHASE_THIRD_PLATFORM))
				currentPhase = PHASE_THIRD_PLATFORM;

			if (events.IsInPhase(PHASE_TWO))
				currentPhase = PHASE_TWO;

			if (HealthBelowPct(80) && events.IsInPhase(PHASE_FIRST_PLATFORM))
			{
				events.SetPhase(PHASE_SECOND_PLATFORM);
				events.ScheduleEvent(EVENT_PHASE_SECON_PLAT, 7000, 0, PHASE_SECOND_PLATFORM);
				RampChange(true);
				SecondPlatform(true);
			}

			if (HealthBelowPct(60) && events.IsInPhase(PHASE_SECOND_PLATFORM))
			{
				events.SetPhase(PHASE_THIRD_PLATFORM);
				events.ScheduleEvent(EVENT_PHASE_THIRD_PLAT, 5000, 0, PHASE_THIRD_PLATFORM);
				RampChange(true);
				ThirdPlatform(true);
			}

			if (HealthBelowPct(40) && events.IsInPhase(PHASE_THIRD_PLATFORM))
			{
				CenterChange();
				events.SetPhase(PHASE_TWO);
				events.ScheduleEvent(EVENT_INHALE,           urand(5000, 10000),  0, PHASE_TWO);
				events.ScheduleEvent(EVENT_EXHALE,           urand(12000, 15000), 0, PHASE_TWO);
				events.ScheduleEvent(EVENT_FORCE_AND_WEARVE, urand(42000, 45000), 0, PHASE_TWO);
				events.ScheduleEvent(EVENT_ATTENUATION,      urand(42000, 45000), 0, PHASE_TWO);
				events.ScheduleEvent(EVENT_CONVERT,          urand(58000, 60000), 0, PHASE_TWO);
			}

			while (uint32 eventId = events.ExecuteEvent())
			{
				switch (eventId)
				{
					case EVENT_TALK_INTRO_TWO:
					{
						Talk(TALK_INTRO_TWO);
						events.ScheduleEvent(EVENT_TALK_INTRO_THREE, 5000);
						break;
					}

					case EVENT_TALK_INTRO_THREE:
					{
						Talk(TALK_INTRO_THREE);
						events.ScheduleEvent(EVENT_TALK_INTRO_FOUR, 5000);
						break;
					}

					case EVENT_TALK_INTRO_FOUR:
					{
						Talk(TALK_INTRO_FOUR);
						events.ScheduleEvent(EVENT_TALK_INTRO_FIVE, 6000);
						break;
					}

					case EVENT_TALK_INTRO_FIVE:
					{
						Talk(TALK_INTRO_FIVE);
						events.ScheduleEvent(EVENT_TALK_INTRO_SIX, 9000);
						break;
					}

					case EVENT_TALK_INTRO_SIX:
					{
						Talk(TALK_INTRO_SIX);
						me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
						me->SetReactState(ReactStates::REACT_PASSIVE);
						break;
					}

					case EVENT_PHASE_FIRST_PLAT:
					{
						events.Reset();
						events.ScheduleEvent(EVENT_INHALE,           urand(5000, 10000),  0, PHASE_FIRST_PLATFORM);
						events.ScheduleEvent(EVENT_EXHALE,           urand(12000, 15000), 0, PHASE_FIRST_PLATFORM);
						events.ScheduleEvent(EVENT_FORCE_AND_WEARVE, urand(42000, 45000), 0, PHASE_FIRST_PLATFORM);
						break;
					}

					case EVENT_PHASE_SECON_PLAT:
					{
						events.Reset();
						events.ScheduleEvent(EVENT_INHALE,      urand(5000, 10000),  0, PHASE_SECOND_PLATFORM);
						events.ScheduleEvent(EVENT_EXHALE,      urand(12000, 15000), 0, PHASE_SECOND_PLATFORM);
						events.ScheduleEvent(EVENT_ATTENUATION, urand(42000, 45000), 0, PHASE_SECOND_PLATFORM);
						break;
					}

					case EVENT_PHASE_THIRD_PLAT:
					{
						events.Reset();
						events.ScheduleEvent(EVENT_INHALE,      urand(5000, 10000),  0, PHASE_THIRD_PLATFORM);
						events.ScheduleEvent(EVENT_EXHALE,      urand(12000, 15000), 0, PHASE_THIRD_PLATFORM);
						events.ScheduleEvent(EVENT_CONVERT,     urand(58000, 60000), 0, PHASE_THIRD_PLATFORM);
						break;
					}

					case EVENT_INHALE:
					{
						events.ScheduleEvent(EVENT_INHALE, urand(5000, 10000), 0, currentPhase);
						break;
					}

					case EVENT_EXHALE:
					{
						events.ScheduleEvent(EVENT_EXHALE, urand(12000, 15000), 0, currentPhase);
						break;
					}

					case EVENT_FORCE_AND_WEARVE:
					{
						events.ScheduleEvent(EVENT_FORCE_AND_WEARVE, urand(43000, 45000), 0, currentPhase);
						break;
					}

					case EVENT_ATTENUATION:
					{
						events.ScheduleEvent(EVENT_ATTENUATION, urand(43000, 45000), 0, currentPhase);
						break;
					}

					case EVENT_CONVERT:
					{
						events.ScheduleEvent(EVENT_CONVERT, urand(58000, 60000), 0, currentPhase);
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