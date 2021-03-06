/*
 * Copyright (C) 2013-2016 JadeCore <https://www.jadecore.tk/>
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2011-2016 Project SkyFire <http://www.projectskyfire.org/>
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

/*
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "npc_pet_hunter_".
 */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"

enum HunterSpells
{
    SPELL_HUNTER_CRIPPLING_POISON       = 30981,   // Viper
    SPELL_HUNTER_DEADLY_POISON          = 34655,   // Venomous Snake
    SPELL_HUNTER_MIND_NUMBING_POISON    = 25810    // Viper
};

enum HunterCreatures
{
    NPC_HUNTER_VIPER                    = 19921
};

class npc_pet_hunter_snake_trap : public CreatureScript
{
    public:
        npc_pet_hunter_snake_trap() : CreatureScript("npc_pet_hunter_snake_trap") { }

        struct npc_pet_hunter_snake_trapAI : public ScriptedAI
        {
            npc_pet_hunter_snake_trapAI(Creature* creature) : ScriptedAI(creature) { }

            void EnterCombat(Unit* /*who*/) { }

            void Reset()
            {
                _spellTimer = 0;

                CreatureTemplate const* Info = me->GetCreatureTemplate();

                _isViper = Info->Entry == NPC_HUNTER_VIPER ? true : false;

                me->SetMaxHealth(uint32(107 * (me->getLevel() - 40) * 0.025f));
                // Add delta to make them not all hit the same time
                uint32 delta = (rand() % 7) * 100;
                me->SetStatFloatValue(UNIT_FIELD_ATTACK_ROUND_BASE_TIME, float(Info->baseattacktime + delta));
                me->SetStatFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER, float(Info->attackpower));

                // Start attacking attacker of owner on first ai update after spawn - move in line of sight may choose better target
                if (!me->GetVictim() && me->IsSummon())
                    if (Unit* Owner = me->ToTempSummon()->GetSummoner())
                        if (Owner->getAttackerForHelper())
                            AttackStart(Owner->getAttackerForHelper());
            }

            // Redefined for random target selection:
            void MoveInLineOfSight(Unit* who)
            {
                if (!me->GetVictim() && me->CanCreatureAttack(who))
                {
                    if (me->GetDistanceZ(who) > CREATURE_Z_ATTACK_RANGE)
                        return;

                    float attackRadius = me->GetAttackDistance(who);
                    if (me->IsWithinDistInMap(who, attackRadius) && me->IsWithinLOSInMap(who))
                    {
                        if (!(rand() % 5))
                        {
                            me->setAttackTimer(BASE_ATTACK, (rand() % 10) * 100);
                            _spellTimer = (rand() % 10) * 100;
                            AttackStart(who);
                        }
                    }
                }
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->GetVictim()->HasBreakableByDamageCrowdControlAura(me))
                {
                    me->InterruptNonMeleeSpells(false);
                    return;
                }

                if (_spellTimer <= diff)
                {
                    if (_isViper) // Viper
                    {
                        if (urand(0, 2) == 0) //33% chance to cast
                        {
                            uint32 spell;
                            if (urand(0, 1) == 0)
                                spell = SPELL_HUNTER_MIND_NUMBING_POISON;
                            else
                                spell = SPELL_HUNTER_CRIPPLING_POISON;

                            DoCastVictim(spell);
                        }

                        _spellTimer = 3000;
                    }
                    else // Venomous Snake
                    {
                        if (urand(0, 2) == 0) // 33% chance to cast
                            DoCastVictim(SPELL_HUNTER_DEADLY_POISON);
                        _spellTimer = 1500 + (rand() % 5) * 100;
                    }
                }
                else
                    _spellTimer -= diff;

                DoMeleeAttackIfReady();
            }

        private:
            bool _isViper;
            uint32 _spellTimer;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_pet_hunter_snake_trapAI(creature);
        }
};

class npc_pet_hunter_murder_of_crows : public CreatureScript
{
    public:
        npc_pet_hunter_murder_of_crows() : CreatureScript("npc_pet_hunter_murder_of_crows") { }

        struct npc_pet_hunter_murder_of_crowsAI : public ScriptedAI
        {
            npc_pet_hunter_murder_of_crowsAI(Creature *creature) : ScriptedAI(creature)
            {
                me->SetReactState(REACT_DEFENSIVE);
            }

			void UpdateAI(uint32 diff)
            {
                if (me->GetReactState() != REACT_DEFENSIVE)
                    me->SetReactState(REACT_DEFENSIVE);

                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_pet_hunter_murder_of_crowsAI(creature);
        }
};

class npc_pet_hunter_dire_beast : public CreatureScript
{
    public:
        npc_pet_hunter_dire_beast() : CreatureScript("npc_pet_hunter_dire_beast") { }

        struct npc_pet_hunter_dire_beastAI : public ScriptedAI
        {
            npc_pet_hunter_dire_beastAI(Creature *creature) : ScriptedAI(creature) {}

            void Reset()
            {
                me->SetReactState(REACT_DEFENSIVE);

                if (me->GetOwner())
                    if (me->GetOwner()->GetVictim() || me->GetOwner()->getAttackerForHelper())
                        AttackStart(me->GetOwner()->GetVictim() ? me->GetOwner()->GetVictim() : me->GetOwner()->getAttackerForHelper());
            }

			void UpdateAI(uint32 diff)
            {
                if (me->GetReactState() != REACT_DEFENSIVE)
                    me->SetReactState(REACT_DEFENSIVE);

                if (!UpdateVictim())
                {
                    if (Unit* owner = me->GetOwner())
                        if (Unit* newVictim = owner->getAttackerForHelper())
                            AttackStart(newVictim);

                    return;
                }

                if (me->GetVictim())
                    if (Unit* owner = me->GetOwner())
                        if (Unit* ownerVictim = owner->getAttackerForHelper())
                            if (me->GetVictim()->GetGUID() != ownerVictim->GetGUID())
                                AttackStart(ownerVictim);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_pet_hunter_dire_beastAI(creature);
        }
};

void AddSC_hunter_pet_scripts()
{
    new npc_pet_hunter_snake_trap();
    new npc_pet_hunter_murder_of_crows();
    new npc_pet_hunter_dire_beast();
}