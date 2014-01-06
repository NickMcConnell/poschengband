#include "angband.h"

#include <assert.h>

static const char * _desc = 
    "One of the mightier undead creatures, the Vampire is an awe-inspiring sight. Yet this "
    "dread creature has a serious weakness: the bright rays of sun are its bane, and it "
    "will need to flee the surface to the deep recesses of earth until the sun finally "
    "sets. Darkness, on the other hand, (eventually) only makes the Vampire stronger. "
    "As undead, the Vampire has a firm hold on its life force, and resists nether attacks. "
    "The Vampire also resists cold and poison based attacks. It is, however, susceptible to its "
    "perpetual hunger for fresh blood, which can only be satiated by sucking the blood "
    "from a nearby monster.\n \n"
    "The Vampire is a monster race and cannot choose a normal class. Instead, the vampire gains "
    "access to various dark powers as they evolve. Of course, they gain a vampiric bite at a "
    "very early stage, as they must use this power to feed on the living. Killing humans with this "
    "power also is a means of perpetuating the vampire species, and many are the servants of "
    "true prince of darkness! Vampires are also rumored to have limited shapeshifting abilities "
    "and a powerful, hypnotic gaze.";

bool vampiric_drain_hack = FALSE;

/******************************************************************************
 *                  25                35              45
 * Vampire: Vampire -> Master Vampire -> Vampire Lord -> Elder Vampire
 ******************************************************************************/
static void _birth(void) 
{ 
    object_type    forge;

    p_ptr->current_r_idx = MON_VAMPIRE;
    
    object_prep(&forge, lookup_kind(TV_SOFT_ARMOR, SV_LEATHER_SCALE_MAIL));
    add_outfit(&forge);

    object_prep(&forge, lookup_kind(TV_SWORD, SV_DAGGER));
    forge.name2 = EGO_WEAPON_DEATH;
    add_outfit(&forge);
}

static void _gain_level(int new_level) 
{
    if (p_ptr->current_r_idx == MON_VAMPIRE && new_level >= 25)
    {
        p_ptr->current_r_idx = MON_MASTER_VAMPIRE;
        msg_print("You have evolved into a Master Vampire.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_MASTER_VAMPIRE && new_level >= 35)
    {
        p_ptr->current_r_idx = MON_VAMPIRE_LORD;
        msg_print("You have evolved into a Vampire Lord.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_VAMPIRE_LORD && new_level >= 45)
    {
        p_ptr->current_r_idx = MON_ELDER_VAMPIRE;
        msg_print("You have evolved into an Elder Vampire.");
        p_ptr->redraw |= PR_MAP;
    }
}

/******************************************************************************
 * Vampire Bite
 ******************************************************************************/
static void _calc_innate_attacks(void) 
{
    innate_attack_t    a = {0};

    a.dd = 1 + p_ptr->lev / 12;
    a.ds = 5 + p_ptr->lev / 10;
    a.weight = 100;
    a.to_h = p_ptr->lev/2;
    a.to_d = p_ptr->lev/2;

    a.effect[0] = GF_OLD_DRAIN;
    a.blows = 100;

    a.msg = "You bite %s.";
    a.name = "Bite";

    p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
}

/******************************************************************************
 * Powers
 ******************************************************************************/
static int _bite_amt(void)
{
    int l = p_ptr->lev;
    return l + l*l/25 + l*l*l*3/2500; /* 50 + 100 + 150 = 300 */
}
static void _bite_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Vampiric Bite");
        break;
    case SPELL_DESC:
        var_set_string(res, "As a vampire, you must feed on fresh blood in order to sustain your unlife!");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _bite_amt()));
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (d_info[dungeon_type].flags1 & DF1_NO_MELEE)
        {
            msg_print("Something prevents you from attacking.");
            return;
        }
        else
        {
            int x = 0, y = 0, amt, food, m_idx = 0;
            int dir = 0;

            if (use_old_target && target_okay())
            {
                y = target_row;
                x = target_col;
                m_idx = cave[y][x].m_idx;
                if (m_idx)
                {
                    if (m_list[m_idx].cdis > 1)
                        m_idx = 0;
                    else
                        dir = 5;
                }
            }

            if (!m_idx)
            {
                if (!get_rep_dir2(&dir)) return;
                if (dir == 5) return;
                y = py + ddy[dir];
                x = px + ddx[dir];
                m_idx = cave[y][x].m_idx;

                if (!m_idx)
                {
                    msg_print("There is no monster there.");
                    return;
                }
            }

            var_set_bool(res, TRUE);

            msg_print("You grin and bare your fangs...");
            amt = _bite_amt();

            vampiric_drain_hack = TRUE;
            if (project(0, 0, y, x, amt, GF_OLD_DRAIN, PROJECT_STOP | PROJECT_KILL | PROJECT_THRU, -1))
            {
                if (p_ptr->food < PY_FOOD_FULL)
                    hp_player(amt);
                else
                    msg_print("You were not hungry.");

                /* Experimental: Scale the feeding asymptotically. Historically, vampiric feeding
                   was too slow in the early game (low damage) hence tedious. But by the end game,
                   a mere two bites would fill the vampire, rendering the talent rather useless. */
                if (p_ptr->food < PY_FOOD_VAMP_MAX)
                    food = p_ptr->food + (PY_FOOD_VAMP_MAX - p_ptr->food) / 4;
                /* Exceeding PY_FOOD_VAMP_MAX is unlikely, but possible (eg. eating rations of food?!) */
                else if (p_ptr->food < PY_FOOD_MAX)
                    food = p_ptr->food + (PY_FOOD_MAX - p_ptr->food) / 4;
                else
                    food = p_ptr->food + amt;

                assert(food >= p_ptr->food);
                set_food(food);
            }
            else
                msg_print("Yechh. That tastes foul.");
            vampiric_drain_hack = FALSE;
        }
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, MIN(_bite_amt() / 10, 29));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
static int _gaze_power(void)
{
    int power = p_ptr->lev;
    if (p_ptr->lev > 40)
        power += p_ptr->lev - 40;
    power += adj_con_fix[p_ptr->stat_ind[A_CHR]] - 1;
    return power;
}
void _gaze_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Vampiric Gaze");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attempts to dominate an intelligent foe causing stunning, confusion, fear or perhaps even enslavement.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(_gaze_power()));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_aim_dir(&dir)) return;
        fire_ball(GF_DOMINATION, dir, _gaze_power(), 0);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _grasp_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Vampiric Grasp");
        break;
    case SPELL_DESC:
        var_set_string(res, "Pulls a target creature to you.");
        break;
    default:
        teleport_to_spell(cmd, res);
        break;
    }
}

static spell_info _spells[] = 
{
    {  2,  1, 60, _bite_spell },
    {  5,  3, 30, detect_life_spell },
    {  7,  4, 50, polymorph_bat_spell },
    { 10,  5, 30, create_darkness_spell },
    { 15, 12, 40, _gaze_spell },
    { 20, 20, 50, amnesia_spell },
    { 25,  7, 40, nether_bolt_spell },       /* Master Vampire */
    { 25, 10, 50, mind_blast_spell },
    { 25, 20, 60, polymorph_mist_spell },
    { 35, 25, 50, nether_ball_spell },       /* Vampire Lord */
    { 35, 30, 60, _grasp_spell },
    { 45, 50, 80, darkness_storm_II_spell }, /* Elder Vampire */
    { -1, -1, -1, NULL}
};

static int _get_spells(spell_info* spells, int max) 
{
    return get_spells_aux(spells, max, _spells);
}

static caster_info * _caster_info(void) 
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "dark power";
        me.which_stat = A_CHR;
        me.weight = 450;
        init = TRUE;
    }
    return &me;
}

/******************************************************************************
 * Bonuses
 ******************************************************************************/
static void _calc_bonuses(void) 
{
    p_ptr->align -= 200;

    res_add(RES_DARK);
    res_add(RES_NETHER);
    res_add(RES_COLD);
    res_add(RES_POIS);
    res_add_vuln(RES_LITE);
    p_ptr->hold_life = TRUE;
    p_ptr->see_nocto = TRUE;

    if (p_ptr->lev >= 35)
    {
        p_ptr->levitation = TRUE;
        p_ptr->pspeed += 1;
        p_ptr->regenerate = TRUE;
    }

    if (p_ptr->lev >= 45)
    {
        p_ptr->pspeed += 2;
        res_add_immune(RES_DARK);
    }
}

static void _get_flags(u32b flgs[TR_FLAG_SIZE]) 
{
    add_flag(flgs, TR_RES_NETHER);
    add_flag(flgs, TR_RES_COLD);
    add_flag(flgs, TR_RES_POIS);
    add_flag(flgs, TR_RES_DARK);
    add_flag(flgs, TR_HOLD_LIFE);
    if (p_ptr->lev >= 35)
    {
        add_flag(flgs, TR_LEVITATION);
        add_flag(flgs, TR_SPEED);
        add_flag(flgs, TR_REGEN);
    }
}

static void _get_immunities(u32b flgs[TR_FLAG_SIZE])
{
    if (p_ptr->lev >= 45)
    {
        add_flag(flgs, TR_RES_DARK);
    }
}

static void _get_vulnerabilities(u32b flgs[TR_FLAG_SIZE])
{
    add_flag(flgs, TR_RES_LITE);
}

/******************************************************************************
 * Public
 ******************************************************************************/
race_t *mon_vampire_get_race_t(void)
{
    static race_t me = {0};
    static bool init = FALSE;
    static cptr titles[4] =  {"Vampire", "Master Vampire", "Vampire Lord", "Elder Vampire"};    
    int         rank = 0;

    if (p_ptr->lev >= 25) rank++;
    if (p_ptr->lev >= 35) rank++;
    if (p_ptr->lev >= 45) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  37,  36,   6,  32,  25,  60,  35};
    skills_t xs = {  7,  12,  10,   1,   0,   0,  21,  11};

        me.name = "Vampire";
        me.desc = _desc;

        me.skills = bs;
        me.extra_skills = xs;

        me.base_hp = 20;
        me.exp = 250;
        me.infra = 5;

        me.birth = _birth;
        me.gain_level = _gain_level;

        me.get_spells = _get_spells;
        me.caster_info = _caster_info;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.get_immunities = _get_immunities;
        me.get_vulnerabilities = _get_vulnerabilities;
        /* I'm debating this: me.calc_innate_attacks = _calc_innate_attacks; */

        me.flags = RACE_IS_NONLIVING | RACE_IS_UNDEAD | RACE_IS_MONSTER;
        me.pseudo_class_idx = CLASS_ROGUE;

        me.boss_r_idx = MON_VLAD;

        init = TRUE;
    }

    me.subname = titles[rank];
    me.stats[A_STR] =  1 + rank;
    me.stats[A_INT] =  1 + (rank+1)/2;
    me.stats[A_WIS] = -1 - rank;
    me.stats[A_DEX] =  0 + rank;
    me.stats[A_CON] = -1;
    me.stats[A_CHR] =  1 + 3*rank/2;
    me.life = 90 + 3*rank;

    me.equip_template = mon_get_equip_template();

    return &me;
}

/*
N:432:Vampire
G:V:W
I:110:25d12:20:45:10:170
W:27:1:70:175:2000:520
B:HIT:HURT:1d6
B:HIT:HURT:1d6
B:BITE:EXP_VAMP:1d8
B:TOUCH:DR_MANA:1d10
F:FORCE_SLEEP | 
F:COLD_BLOOD | DROP_60 | DROP_1D2 | 
F:OPEN_DOOR | BASH_DOOR | REGENERATE | CAN_FLY | RES_NETH |
F:EVIL | UNDEAD | RES_COLD | IM_POIS | RES_DARK | HURT_LITE | NO_CONF | NO_SLEEP
S:1_IN_9 | 
S:TELE_TO | HOLD | SCARE | CAUSE_2 | MIND_BLAST | FORGET | DARKNESS

N:520:Master vampire
G:V:s
I:110:34d10:20:60:10:170
W:34:3:80:750:5000:623
B:HIT:HURT:1d10
B:BITE:EXP_VAMP:1d11
B:BITE:EXP_VAMP:1d11
B:TOUCH:DR_MANA:2d8
F:FORCE_SLEEP | FORCE_MAXHP | 
F:DROP_4D2 | 
F:COLD_BLOOD | OPEN_DOOR | BASH_DOOR | REGENERATE | CAN_FLY | RES_NETH |
F:EVIL | UNDEAD | RES_COLD | IM_POIS | RES_DARK | HURT_LITE | NO_CONF | NO_SLEEP
S:1_IN_6 | 
S:TELE_TO | HOLD | CONF | SCARE | CAUSE_3 | MIND_BLAST | FORGET | 
S:DARKNESS | BO_NETH

N:623:Vampire lord
G:V:b
I:120:16d100:20:70:10:170
W:39:3:999:1800:20000:1058
B:HIT:HURT:1d6
B:HIT:HURT:1d6
B:BITE:EXP_VAMP:4d6
B:TOUCH:DR_MANA:4d6
F:FORCE_SLEEP | FORCE_MAXHP | CAN_FLY |
F:DROP_60 | DROP_4D2 |
F:COLD_BLOOD | OPEN_DOOR | BASH_DOOR | REGENERATE | RES_TELE |
F:EVIL | UNDEAD | RES_COLD | IM_POIS | RES_NETH | RES_DARK | HURT_LITE |
F:NO_CONF | NO_SLEEP
S:1_IN_7 | 
S:BLIND | HOLD | SCARE | CAUSE_3 | CAUSE_4 | DRAIN_MANA | 
S:BRAIN_SMASH | DARKNESS | BO_NETH

N:1058:Elder vampire
G:V:B
I:120:34d100:20:90:10:170
W:54:3:999:7500:0:0
B:HIT:HURT:4d6
B:HIT:HURT:4d6
B:BITE:EXP_VAMP:6d8
B:BITE:DR_MANA:6d8
F:FORCE_SLEEP | FORCE_MAXHP | CAN_FLY |
F:DROP_60 | DROP_4D2 |
F:COLD_BLOOD | OPEN_DOOR | BASH_DOOR | REGENERATE | RES_TELE | 
F:EVIL | UNDEAD | IM_COLD | IM_POIS | RES_NETH | RES_DARK | HURT_LITE | 
F:NO_CONF | NO_SLEEP |
S:1_IN_3 | 
S:BLIND | HOLD | SCARE | CAUSE_3 | CAUSE_4 | DRAIN_MANA | BA_NETH
S:BRAIN_SMASH | DARKNESS | BO_NETH | S_UNDEAD

*/