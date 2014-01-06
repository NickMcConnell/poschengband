#include "angband.h"

/****************************************************************
 * Bat
 ****************************************************************/
static void _bat_calc_innate_attacks(void) 
{
    innate_attack_t    a = {0};

    a.dd = 1;
    a.ds = 4;
    a.weight = 50;
    a.to_h = p_ptr->lev/2;

    a.effect[0] = GF_OLD_DRAIN;
    calc_innate_blows(&a, 400);

    a.msg = "You bite %s.";
    a.name = "Bite";

    p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
}
static void _bat_calc_bonuses(void)
{
    p_ptr->levitation = TRUE;
    p_ptr->see_inv = TRUE;
    p_ptr->regenerate = TRUE;
    res_add(RES_DARK);
    res_add(RES_COLD);
    res_add(RES_POIS);
    p_ptr->see_nocto = TRUE;
    p_ptr->pspeed += 5 + p_ptr->lev * 3 / 10;
    p_ptr->hold_life = TRUE;
}
static void _bat_get_flags(u32b flgs[TR_FLAG_SIZE])
{
    add_flag(flgs, TR_LEVITATION);
    add_flag(flgs, TR_SEE_INVIS);
    add_flag(flgs, TR_REGEN);
    add_flag(flgs, TR_SPEED);
    add_flag(flgs, TR_RES_DARK);
    add_flag(flgs, TR_RES_COLD);
    add_flag(flgs, TR_RES_POIS);
    add_flag(flgs, TR_HOLD_LIFE);
}
race_t *bat_get_race_t(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Vampire Bat";
        me.desc = "";

        me.stats[A_STR] = -5;
        me.stats[A_INT] =  0;
        me.stats[A_WIS] =  0;
        me.stats[A_DEX] =  4;
        me.stats[A_CON] = -5;
        me.stats[A_CHR] = -5;
        
        me.skills.dis =  0;
        me.skills.dev =  0;
        me.skills.sav =  0;
        me.skills.stl = 12;
        me.skills.srh = 15;
        me.skills.fos = 15;
        me.skills.thn = -20;
        me.skills.thb = -20;

        me.life = 75;
        me.base_hp = 10;
        me.exp = 75;
        me.infra = 10;

        me.calc_innate_attacks = _bat_calc_innate_attacks;
        me.calc_bonuses = _bat_calc_bonuses;
        me.get_flags = _bat_get_flags;

        me.equip_template = &b_info[r_info[MON_VAMPIRE_BAT].body.body_idx];
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Clay-Golem
 ****************************************************************/
static void _clay_golem_calc_bonuses(void)
{
    p_ptr->free_act = TRUE;
    p_ptr->hold_life = TRUE;
    p_ptr->to_a += 10;
    p_ptr->dis_to_a += 10;
}
static void _clay_golem_get_flags(u32b flgs[TR_FLAG_SIZE])
{
    add_flag(flgs, TR_FREE_ACT);
    add_flag(flgs, TR_HOLD_LIFE);
}
race_t *clay_golem_get_race_t(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Clay-Golem";
        me.desc = "";

        me.stats[A_STR] =  2;
        me.stats[A_INT] =  0;
        me.stats[A_WIS] =  0;
        me.stats[A_DEX] =  0;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] = -2;
        
        me.skills.dis = -5;
        me.skills.dev = -5;
        me.skills.sav = 8;
        me.skills.stl = -2;
        me.skills.srh = -2;
        me.skills.fos = 5;
        me.skills.thn = 20;
        me.skills.thb = 0;

        me.life = 102;
        me.base_hp = 22;
        me.exp = 200;
        me.infra = 2;

        me.flags = RACE_IS_NONLIVING;

        me.calc_bonuses = _clay_golem_calc_bonuses;
        me.get_flags = _clay_golem_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Colossus
 ****************************************************************/
static void _colossus_calc_bonuses(void)
{
    p_ptr->free_act = TRUE;
    p_ptr->see_inv = TRUE;
    p_ptr->hold_life = TRUE;
    res_add(RES_POIS);
    res_add(RES_SHARDS);
    res_add(RES_SOUND);
    res_add(RES_DISEN);
    p_ptr->reflect = TRUE;
    p_ptr->pspeed -= 5;
    p_ptr->to_a += 40;
    p_ptr->dis_to_a += 40;
}
static void _colossus_get_flags(u32b flgs[TR_FLAG_SIZE])
{
    add_flag(flgs, TR_FREE_ACT);
    add_flag(flgs, TR_SEE_INVIS);
    add_flag(flgs, TR_HOLD_LIFE);
    add_flag(flgs, TR_RES_POIS);
    add_flag(flgs, TR_SPEED);
    add_flag(flgs, TR_RES_SHARDS);
    add_flag(flgs, TR_REFLECT);
    add_flag(flgs, TR_RES_SOUND);
    add_flag(flgs, TR_RES_DISEN);
}
race_t *colossus_get_race_t(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Colossus";
        me.desc = "";

        me.stats[A_STR] =  7;
        me.stats[A_INT] =  2;
        me.stats[A_WIS] =  2;
        me.stats[A_DEX] = -4;
        me.stats[A_CON] =  7;
        me.stats[A_CHR] =  4;
        
        me.skills.dis =  0;
        me.skills.dev =  0;
        me.skills.sav = 35;
        me.skills.stl = -4;
        me.skills.srh = -2;
        me.skills.fos = 5;
        me.skills.thn = 90;
        me.skills.thb = -20;

        me.life = 115;
        me.base_hp = 30;
        me.exp = 1000;
        me.infra = 5;
        me.flags = RACE_IS_NONLIVING;

        me.calc_bonuses = _colossus_calc_bonuses;
        me.get_flags = _colossus_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Demon (cf Polymorph Demon)
 ****************************************************************/
static power_info _demon_powers[] =
{
    { A_CON, {15, 10, 70, demon_breath_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static int _demon_get_powers(spell_info* spells, int max)
{
    return get_powers_aux(spells, max, _demon_powers);
}
static void _demon_calc_bonuses(void)
{
    p_ptr->hold_life = TRUE;
    res_add(RES_CHAOS);
    res_add(RES_NETHER);
    res_add(RES_FIRE);
    res_add(RES_FIRE);
    p_ptr->see_inv = TRUE;
    p_ptr->pspeed += 3;
    p_ptr->redraw |= PR_STATUS;
    p_ptr->to_a += 10;
    p_ptr->dis_to_a += 10;
    p_ptr->align -= 200;
}
static void _demon_get_flags(u32b flgs[TR_FLAG_SIZE])
{
    add_flag(flgs, TR_HOLD_LIFE);
    add_flag(flgs, TR_RES_CHAOS);
    add_flag(flgs, TR_RES_NETHER);
    add_flag(flgs, TR_RES_FIRE);
    add_flag(flgs, TR_SEE_INVIS);
    add_flag(flgs, TR_SPEED);
}
race_t *demon_get_race_t(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Demon";
        me.desc = "";

        me.stats[A_STR] =  5;
        me.stats[A_INT] =  3;
        me.stats[A_WIS] =  2;
        me.stats[A_DEX] =  3;
        me.stats[A_CON] =  4;
        me.stats[A_CHR] =  3;
        
        me.skills.dis = -5;
        me.skills.dev = 18;
        me.skills.sav = 20;
        me.skills.stl = -2;
        me.skills.srh =  3;
        me.skills.fos = 10;
        me.skills.thn = 40;
        me.skills.thb = 20;

        me.life = 106;
        me.base_hp = 24;
        me.exp = 500;
        me.infra = 5;
        me.flags = RACE_IS_NONLIVING | RACE_IS_DEMON;

        me.calc_bonuses = _demon_calc_bonuses;
        me.get_powers = _demon_get_powers;
        me.get_flags = _demon_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Demon-Lord (cf Polymorph Demon-Lord)
 ****************************************************************/
static power_info _demon_lord_powers[] =
{
    { A_CON, {15, 10, 70, demon_breath_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static int _demon_lord_get_powers(spell_info* spells, int max)
{
    return get_powers_aux(spells, max, _demon_lord_powers);
}
static void _demon_lord_calc_bonuses(void)
{
    p_ptr->hold_life = TRUE;
    res_add(RES_CHAOS);
    res_add(RES_NETHER);
    res_add_immune(RES_FIRE);
    res_add(RES_ACID);
    res_add(RES_COLD);
    res_add(RES_ELEC);
    res_add(RES_POIS);
    res_add(RES_CONF);
    res_add(RES_DISEN);
    res_add(RES_NEXUS);
    res_add(RES_FEAR);
    p_ptr->sh_fire = TRUE;
    p_ptr->see_inv = TRUE;
    p_ptr->telepathy = TRUE;
    p_ptr->levitation = TRUE;
    p_ptr->kill_wall = TRUE;
    p_ptr->pspeed += 5;
    p_ptr->to_a += 20;
    p_ptr->dis_to_a += 20;
    p_ptr->align -= 200;
}
static void _demon_lord_get_flags(u32b flgs[TR_FLAG_SIZE])
{
    add_flag(flgs, TR_HOLD_LIFE);
    add_flag(flgs, TR_RES_CHAOS);
    add_flag(flgs, TR_RES_NETHER);
    add_flag(flgs, TR_RES_FIRE);
    add_flag(flgs, TR_RES_COLD);
    add_flag(flgs, TR_RES_ELEC);
    add_flag(flgs, TR_RES_ACID);
    add_flag(flgs, TR_RES_POIS);
    add_flag(flgs, TR_RES_CONF);
    add_flag(flgs, TR_RES_DISEN);
    add_flag(flgs, TR_RES_NEXUS);
    add_flag(flgs, TR_RES_FEAR);
    add_flag(flgs, TR_IM_FIRE);
    add_flag(flgs, TR_SH_FIRE);
    add_flag(flgs, TR_SEE_INVIS);
    add_flag(flgs, TR_TELEPATHY);
    add_flag(flgs, TR_LEVITATION);
    add_flag(flgs, TR_SPEED);
}
static void _demon_lord_get_immunities(u32b flgs[TR_FLAG_SIZE])
{
    add_flag(flgs, TR_RES_FIRE);
}
race_t *demon_lord_get_race_t(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Demon-Lord";
        me.desc = "";

        me.stats[A_STR] = 20;
        me.stats[A_INT] = 20;
        me.stats[A_WIS] = 20;
        me.stats[A_DEX] = 20;
        me.stats[A_CON] = 20;
        me.stats[A_CHR] = 20;
        
        me.skills.dis = 20;
        me.skills.dev = 20;
        me.skills.sav = 25;
        me.skills.stl = -2;
        me.skills.srh =  3;
        me.skills.fos = 10;
        me.skills.thn = 70;
        me.skills.thb = 40;

        me.life = 110;
        me.base_hp = 28;
        me.exp = 1500;
        me.infra = 20;
        me.flags = RACE_IS_NONLIVING | RACE_IS_DEMON;

        me.calc_bonuses = _demon_lord_calc_bonuses;
        me.get_powers = _demon_lord_get_powers;
        me.get_flags = _demon_lord_get_flags;
        me.get_immunities = _demon_lord_get_immunities;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Iron-Golem
 ****************************************************************/
static void _iron_golem_calc_bonuses(void)
{
    p_ptr->free_act = TRUE;
    p_ptr->see_inv = TRUE;
    p_ptr->hold_life = TRUE;
    res_add(RES_POIS);
    p_ptr->pspeed -= 1;
    p_ptr->to_a += 15;
    p_ptr->dis_to_a += 15;
}
static void _iron_golem_get_flags(u32b flgs[TR_FLAG_SIZE])
{
    add_flag(flgs, TR_FREE_ACT);
    add_flag(flgs, TR_SEE_INVIS);
    add_flag(flgs, TR_HOLD_LIFE);
    add_flag(flgs, TR_RES_POIS);
    add_flag(flgs, TR_SPEED);
}
race_t *iron_golem_get_race_t(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Iron-Golem";
        me.desc = "";

        me.stats[A_STR] =  3;
        me.stats[A_INT] =  0;
        me.stats[A_WIS] =  0;
        me.stats[A_DEX] = -1;
        me.stats[A_CON] =  3;
        me.stats[A_CHR] = -2;
        
        me.skills.dis = -5;
        me.skills.dev = -5;
        me.skills.sav = 15;
        me.skills.stl = -2;
        me.skills.srh = -2;
        me.skills.fos = 5;
        me.skills.thn = 30;
        me.skills.thb = -5;

        me.life = 105;
        me.base_hp = 24;
        me.exp = 250;
        me.infra = 3;
        me.flags = RACE_IS_NONLIVING;

        me.calc_bonuses = _iron_golem_calc_bonuses;
        me.get_flags = _iron_golem_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Mangy Leper
 ****************************************************************/
race_t *mangy_leper_get_race_t(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Mangy Leper";
        me.desc = "Mangy Lepers are humans who have contracted a horrible wasting disease. "
                    "You cannot help but feel disgusted as your body rots before your very "
                    "eyes.";
        
        me.stats[A_STR] = -1;
        me.stats[A_INT] =  0;
        me.stats[A_WIS] =  0;
        me.stats[A_DEX] = -1;
        me.stats[A_CON] = -3;
        me.stats[A_CHR] = -3;
        
        me.skills.dis = 0;
        me.skills.dev = 0;
        me.skills.sav = 0;
        me.skills.stl = 0;
        me.skills.srh = 0;
        me.skills.fos = 10;
        me.skills.thn = 0;
        me.skills.thb = 0;

        me.life = 88;
        me.base_hp = 10;
        me.exp = 100;
        me.infra = 0;

        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Mist
 ****************************************************************/
static void _mist_calc_bonuses(void)
{
    p_ptr->levitation = TRUE;
    p_ptr->pass_wall = TRUE;
    p_ptr->no_passwall_dam = TRUE;
    p_ptr->see_inv = TRUE;
    p_ptr->see_nocto = TRUE;
    p_ptr->hold_life = TRUE;

    res_add(RES_ACID);
    res_add(RES_COLD);
    res_add(RES_POIS);
    res_add(RES_NETHER);

    p_ptr->magic_resistance = 50;
}
static void _mist_get_flags(u32b flgs[TR_FLAG_SIZE])
{
    add_flag(flgs, TR_LEVITATION);
    add_flag(flgs, TR_SEE_INVIS);
    add_flag(flgs, TR_HOLD_LIFE);

    add_flag(flgs, TR_RES_COLD);
    add_flag(flgs, TR_RES_POIS);
    add_flag(flgs, TR_RES_ACID);
    add_flag(flgs, TR_RES_NETHER);

    add_flag(flgs, TR_MAGIC_RESISTANCE);
}
race_t *mist_get_race_t(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Vampiric Mist";
        me.desc = "You are a cloud of evil, sentient mist. As such you are incorporeal and are "
            "unable to attack enemies directly. Conversely, you are resistant to material damage "
            "and may pass through walls. Probably, you should run away upon assuming this form.";

        me.stats[A_STR] = -3;
        me.stats[A_INT] = -3;
        me.stats[A_WIS] = -3;
        me.stats[A_DEX] = -3;
        me.stats[A_CON] = -3;
        me.stats[A_CHR] = -3;
        
        me.skills.dis =  0;
        me.skills.dev =  0;
        me.skills.sav = 30;
        me.skills.stl = 15;
        me.skills.srh = 15;
        me.skills.fos = 15;
        me.skills.thn = -100;
        me.skills.thb = -100;

        me.life = 80;
        me.base_hp = 15;
        me.exp = 75;
        me.infra = 10;

        me.calc_bonuses = _mist_calc_bonuses;
        me.get_flags = _mist_get_flags;

        me.equip_template = &b_info[r_info[MON_VAMPIRIC_MIST].body.body_idx];
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Mithril-Golem
 ****************************************************************/
static void _mithril_golem_calc_bonuses(void)
{
    p_ptr->free_act = TRUE;
    p_ptr->see_inv = TRUE;
    p_ptr->hold_life = TRUE;
    res_add(RES_POIS);
    res_add(RES_SHARDS);
    p_ptr->reflect = TRUE;
    p_ptr->pspeed -= 2;
    p_ptr->to_a += 20;
    p_ptr->dis_to_a += 20;
}
static void _mithril_golem_get_flags(u32b flgs[TR_FLAG_SIZE])
{
    add_flag(flgs, TR_FREE_ACT);
    add_flag(flgs, TR_SEE_INVIS);
    add_flag(flgs, TR_HOLD_LIFE);
    add_flag(flgs, TR_RES_POIS);
    add_flag(flgs, TR_SPEED);
    add_flag(flgs, TR_RES_SHARDS);
    add_flag(flgs, TR_REFLECT);
}
race_t *mithril_golem_get_race_t(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Mithril-Golem";
        me.desc = "";

        me.stats[A_STR] =  5;
        me.stats[A_INT] =  1;
        me.stats[A_WIS] =  1;
        me.stats[A_DEX] = -3;
        me.stats[A_CON] =  5;
        me.stats[A_CHR] =  2;
        
        me.skills.dis =  0;
        me.skills.dev =  0;
        me.skills.sav = 25;
        me.skills.stl = -3;
        me.skills.srh = -2;
        me.skills.fos = 5;
        me.skills.thn = 50;
        me.skills.thb = -10;

        me.life = 109;
        me.base_hp = 27;
        me.exp = 500;
        me.infra = 4;
        me.flags = RACE_IS_NONLIVING;

        me.calc_bonuses = _mithril_golem_calc_bonuses;
        me.get_flags = _mithril_golem_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Small Kobold
 ****************************************************************/
static void _small_kobold_calc_bonuses(void)
{
    res_add(RES_POIS);
}
static void _small_kobold_get_flags(u32b flgs[TR_FLAG_SIZE])
{
    add_flag(flgs, TR_RES_POIS);
}
race_t *small_kobold_get_race_t(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Small Kobold";
        me.desc = "Small Kobolds are a the runts of the kobold race, often relegated to the "
                    "performance of menial tasks deemed unworthy of their larger, more "
                    "respectable brethren.";

        me.stats[A_STR] =  0;
        me.stats[A_INT] = -2;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  0;
        me.stats[A_CON] = -1;
        me.stats[A_CHR] = -3;
        
        me.skills.dis = -5;
        me.skills.dev = -8;
        me.skills.sav = -3;
        me.skills.stl = -1;
        me.skills.srh =  1;
        me.skills.fos =  8;
        me.skills.thn =  5;
        me.skills.thb = -12;

        me.life = 91;
        me.base_hp = 13;
        me.exp = 50;
        me.infra = 2;

        me.calc_bonuses = _small_kobold_calc_bonuses;
        me.get_flags = _small_kobold_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Wolf
 ****************************************************************/
static power_info _wolf_powers[] = 
{
    { A_DEX, {  1,  1, 30, hound_sniff_spell } },
    { A_DEX, { 10,  0,  0, hound_stalk_spell}},
    { A_DEX, { 15,  0,  0, hound_run_spell}},
    { A_DEX, { 20, 10, 30, hound_leap_spell}},
    {    -1, { -1, -1, -1, NULL}}
};

static int _wolf_get_powers(spell_info* spells, int max) 
{
    return get_powers_aux(spells, max, _wolf_powers);
}
static void _wolf_calc_bonuses(void)
{
    p_ptr->see_nocto = TRUE;
    p_ptr->pspeed += 2 + p_ptr->lev / 10;
}
static void _wolf_get_flags(u32b flgs[TR_FLAG_SIZE])
{
    add_flag(flgs, TR_SPEED);
}
race_t *wolf_get_race_t(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Dire Wolf";
        me.desc = "";

        me.skills.dis =  0;
        me.skills.dev =  0;
        me.skills.sav =  2;
        me.skills.stl =  8;
        me.skills.srh =  5;
        me.skills.fos =  5;
        me.skills.thn = 15;
        me.skills.thb =  0;

        me.life = 100;
        me.base_hp = 22;
        me.exp = 120;
        me.infra = 5;

        me.get_powers = _wolf_get_powers;
        me.calc_innate_attacks = hound_calc_innate_attacks;
        me.calc_bonuses = _wolf_calc_bonuses;
        me.get_flags = _wolf_get_flags;

        me.equip_template = &b_info[17];
        init = TRUE;
    }
    me.stats[A_STR] =  1 + p_ptr->lev/12;
    me.stats[A_INT] = -3;
    me.stats[A_WIS] = -5;
    me.stats[A_DEX] =  2 + p_ptr->lev/15;
    me.stats[A_CON] =  1 + p_ptr->lev/15;
    me.stats[A_CHR] =  0 + p_ptr->lev/25;

    return &me;
}

/****************************************************************
 * Vampire-Lord (cf Polymorph Vampire)
 ****************************************************************/
static power_info _vampire_lord_powers[] =
{
    { A_CON, {2, 1, 60, vampirism_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static int _vampire_lord_get_powers(spell_info* spells, int max)
{
    return get_powers_aux(spells, max, _vampire_lord_powers);
}
static void _vampire_lord_calc_bonuses(void)
{
    res_add(RES_DARK);
    p_ptr->hold_life = TRUE;
    res_add(RES_NETHER);
    res_add(RES_COLD);
    res_add(RES_POIS);
    res_add_vuln(RES_LITE);
    if (p_ptr->pclass != CLASS_NINJA) p_ptr->lite = TRUE;

    p_ptr->see_inv = TRUE;
    p_ptr->pspeed += 3;
    p_ptr->to_a += 10;
    p_ptr->dis_to_a += 10;
}
static void _vampire_lord_get_flags(u32b flgs[TR_FLAG_SIZE])
{
    add_flag(flgs, TR_HOLD_LIFE);
    add_flag(flgs, TR_RES_DARK);
    add_flag(flgs, TR_RES_NETHER);
    if (p_ptr->pclass != CLASS_NINJA) add_flag(flgs, TR_LITE);
    add_flag(flgs, TR_RES_POIS);
    add_flag(flgs, TR_RES_COLD);
    add_flag(flgs, TR_SEE_INVIS);
    add_flag(flgs, TR_SPEED);
}
static void _vampire_lord_get_immunities(u32b flgs[TR_FLAG_SIZE])
{
    add_flag(flgs, TR_RES_DARK);
}
static void _vampire_lord_get_vulnerabilities(u32b flgs[TR_FLAG_SIZE])
{
    add_flag(flgs, TR_RES_LITE);
}
race_t *vampire_lord_get_race_t(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "Vampire";
        me.desc = "";

        me.stats[A_STR] =  4;
        me.stats[A_INT] =  4;
        me.stats[A_WIS] =  1;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  3;
        
        me.skills.dis = 6;
        me.skills.dev = 12;
        me.skills.sav = 8;
        me.skills.stl = 6;
        me.skills.srh = 2;
        me.skills.fos = 12;
        me.skills.thn = 30;
        me.skills.thb = 20;

        me.life = 103;
        me.base_hp = 22;
        me.exp = 300;
        me.infra = 5;
        me.flags = RACE_IS_NONLIVING | RACE_IS_UNDEAD;

        me.calc_bonuses = _vampire_lord_calc_bonuses;
        me.get_powers = _vampire_lord_get_powers;
        me.get_flags = _vampire_lord_get_flags;
        me.get_immunities = _vampire_lord_get_immunities;
        me.get_vulnerabilities = _vampire_lord_get_vulnerabilities;
        init = TRUE;
    }

    return &me;
}

