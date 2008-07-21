/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* ScriptData
SDName: Boss_Murmur
SD%Complete: 75
SDComment: Database should have `RegenHealth`=0 to prevent regen. Also, his shockwave triggered after magnetic pull may be incorrect. Murmur's Touch does not work properly.
SDCategory: Auchindoun, Shadow Labyrinth
EndScriptData */

#include "sc_creature.h"

#define EMOTE_SONIC_BOOM            "draws energy from the air."

#define SPELL_MAGNETIC_PULL         33689
#define SPELL_SONIC_BOOM_PRE        33923
#define SPELL_SONIC_BOOM_CAST       38795
#define SPELL_MURMURS_TOUCH         33711
#define SPELL_RESONANCE             33657
#define SPELL_SHOCKWAVE             33686

struct MANGOS_DLL_DECL boss_murmurAI : public ScriptedAI
{
    boss_murmurAI(Creature *c) : ScriptedAI(c) {Reset();}

    uint32 SonicBoom_Timer;
    uint32 MurmursTouch_Timer;
    uint32 Resonance_Timer;
    uint32 MagneticPull_Timer;
    bool CanSonicBoom;
    bool CanShockWave;
    Unit* target;

    void Reset()
    {
        SonicBoom_Timer = 30000;
        MurmursTouch_Timer = 20000;
        Resonance_Timer = 10000;
        MagneticPull_Timer = 45000;
        CanSonicBoom = false;
        CanShockWave = false;
        target = NULL;

        //database should have `RegenHealth`=0 to prevent regen
        uint32 hp = m_creature->GetMaxHealth()*0.4;
        if( hp )
            m_creature->SetHealth(hp);
    }

    void Aggro(Unit *who) { }

    void AttackStart(Unit* who)
    {
        if (!who)
            return;

        if (who->isTargetableForAttack())
        {
            //Begin attack
            DoStartAttackNoMovement(who);

            if (!InCombat)
            {
                Aggro(who);
                InCombat = true;
            }
        }
    }

    void MoveInLineOfSight(Unit* who)
    {
        if( !m_creature->getVictim() && who->isTargetableForAttack() && ( m_creature->IsHostileTo( who )) && who->isInAccessablePlaceFor(m_creature) )
        {
            if (m_creature->GetDistanceZ(who) > CREATURE_Z_ATTACK_RANGE)
                return;

            float attackRadius = m_creature->GetAttackDistance(who);
            if(m_creature->IsWithinDistInMap(who, attackRadius) && m_creature->IsWithinLOSInMap(who))
            {
                DoStartAttackNoMovement(who);
                who->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);

                if (!InCombat)
                {
                    Aggro(who);
                    InCombat = true;
                }
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!m_creature->SelectHostilTarget() || !m_creature->getVictim() )
            return;

        //SonicBoom_Timer
        if(SonicBoom_Timer < diff)
        {
            if(CanSonicBoom)
            {
                DoCast(m_creature, SPELL_SONIC_BOOM_CAST,true);
                CanSonicBoom = false;
                SonicBoom_Timer = 30000;
            }
            else
            {
                DoTextEmote(EMOTE_SONIC_BOOM,NULL);
                DoCast(m_creature,SPELL_SONIC_BOOM_PRE);
                CanSonicBoom = true;
                SonicBoom_Timer = 5000;
            }
        }else SonicBoom_Timer -= diff;

        //MurmursTouch_Timer
        if(MurmursTouch_Timer < diff)
        {
            /*Unit* target = NULL;
            target = SelectUnit(SELECT_TARGET_RANDOM,0);
            if(target)
                DoCast(target, SPELL_MURMURS_TOUCH);*/
            DoCast(m_creature, SPELL_MURMURS_TOUCH);
            MurmursTouch_Timer = 30000;
        }else MurmursTouch_Timer -= diff;

        //Resonance_Timer
        if(Resonance_Timer < diff)
        {
            if( !m_creature->IsWithinDistInMap(m_creature->getVictim(), ATTACK_DISTANCE) )
                DoCast(m_creature->getVictim(), SPELL_RESONANCE);
            Resonance_Timer = 5000;
        }else Resonance_Timer -= diff;

        //MagneticPull_Timer
        if(MagneticPull_Timer < diff)
        {
            if( !CanShockWave )
            {
                target = SelectUnit(SELECT_TARGET_RANDOM,0);
                if( target )
                    DoCast(target, SPELL_MAGNETIC_PULL);
                MagneticPull_Timer = 2500;
                CanShockWave = true;
            }
            else
            {
                if( target && target->isAlive() )
                    target->CastSpell(target,SPELL_SHOCKWAVE,true);
                MagneticPull_Timer = 35000;
                CanShockWave = false;
                target = NULL;
            }
        }else MagneticPull_Timer -= diff;

        //no meele if preparing for sonic boom
        if(!CanSonicBoom)
            DoMeleeAttackIfReady();
    }
}; 
CreatureAI* GetAI_boss_murmur(Creature *_Creature)
{
    return new boss_murmurAI (_Creature);
}

void AddSC_boss_murmur()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_murmur";
    newscript->GetAI = GetAI_boss_murmur;
    m_scripts[nrscripts++] = newscript;
}
