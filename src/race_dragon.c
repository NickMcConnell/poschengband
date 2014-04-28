#include "angband.h"

#include <assert.h>

static cptr _desc = 
    "Dragons are powerful winged serpents. They are the strongest fighters "
    "with razor sharp claws and a bone crushing bite. Each dragon has a unique "
    "type of breath that becomes more deadly as the dragon grows and matures.\n \n"
    "Due to their non-humanoid bodies, dragons are unable to wear armor, gloves "
    "or boots. However, being creatures of magic, they are able to wear 6 rings. "
    "They can also wear a helmet, a light source, a cloak and an amulet. Because "
    "of these equipment restrictions, dragons may have a difficult time covering "
    "all resistances despite the fact that each dragon has one or more innate "
    "resistances (or even immunities).\n \n"
    "Dragons begin life in a weak form, being very young. As their bodies mature, "
    "their scales grow tough and their claws sharp. Their breath grows more "
    "deadly and they frequently gain additional magical powers and resistances. "
    "All dragons can fly, but younger dragons are not so quick as their elders.\n \n"
    "Dragons are magical creatures, and each dragon may choose to specialize in a "
    "specific type of magic. This magic does not require books to learn and also "
    "has a direct influence on the dragon's stats and skills.";

static dragon_realm_ptr _get_realm(void);

/**********************************************************************
 * Dragon Equipment
 **********************************************************************/
static void _dragon_birth(void) 
{ 
    object_type    forge;

    equip_on_change_race();
    
    object_prep(&forge, lookup_kind(TV_RING, 0));
    forge.name2 = EGO_RING_COMBAT;
    forge.to_h = 3;
    forge.to_d = 3;
    forge.pval = 1;
    add_flag(forge.art_flags, TR_STR);
    add_flag(forge.art_flags, TR_DEX);
    add_outfit(&forge);
}

/**********************************************************************
 * Dragon Breath
 **********************************************************************/

cptr gf_name(int which)
{
    switch (which)
    {
    case GF_FIRE: return "fire";
    case GF_ACID: return "acid";
    case GF_COLD: return "cold";
    case GF_ELEC: return "lightning";
    case GF_POIS: return "poison";
    case GF_LITE: return "light";
    case GF_DARK: return "dark";
    case GF_CONFUSION: return "confusion";
    case GF_NETHER: return "nether";
    case GF_NEXUS: return "nexus";
    case GF_SOUND: return "sound";
    case GF_SHARDS: return "shards";
    case GF_CHAOS: return "chaos";
    case GF_DISENCHANT: return "disenchantment";
    case GF_TIME: return "time";
    case GF_MANA: return "mana";
    case GF_GRAVITY: return "gravity";
    case GF_INERT: return "inerta";
    case GF_PLASMA: return "plasma";
    case GF_FORCE: return "force";
    case GF_NUKE: return "nuke";
    case GF_DISINTEGRATE: return "disintegration";
    case GF_STORM: return "storm";
    }
    return "something";
}

static int _count(int list[])
{
    int i;
    for (i = 0; ; i++)
    {
        if (list[i] == -1) return i;
    }
    /* return 0;  error: missing sentinel ... unreachable */
}

static int _random(int list[])
{
    return list[randint0(_count(list))];
}

static void _effect_menu_fn(int cmd, int which, vptr cookie, variant *res)
{
    int  idx = ((int*)cookie)[which];

    switch (cmd)
    {
    case MENU_TEXT:
        var_set_string(res, format("%^s", gf_name(idx)));
        break;
    }
}

static int _choose_effect(int list[])
{
    int i;
    int ct = _count(list);

    if (REPEAT_PULL(&i))
    {
        if (i >= 0 && i < ct)
            return list[i];
    }

    {
        menu_t menu = { "Choose which effect?", NULL, NULL,
                        _effect_menu_fn, list, ct};
        
        i = menu_choose(&menu);
        if (i >= 0)
        {
            REPEAT_PUSH(i);
            i = list[i];
        }
    }
    return i;
}

static int _get_effect(int list[]) /* va_args is probably a better sig ... */
{
    if (p_ptr->dragon_realm == DRAGON_REALM_BREATH && p_ptr->lev >= 35)
        return _choose_effect(list);
    else
        return _random(list);
}

static int _breath_effect(void)
{
    switch (p_ptr->psubrace)
    {
    case DRAGON_RED: return GF_FIRE;
    case DRAGON_WHITE: return GF_COLD;
    case DRAGON_BLUE: return GF_ELEC;
    case DRAGON_BLACK: return GF_ACID;
    case DRAGON_GREEN: return GF_POIS;
    case DRAGON_BRONZE: return GF_CONFUSION;
    case DRAGON_GOLD: return GF_SOUND;
    case DRAGON_NETHER: 
        if (p_ptr->lev >= 45)
        {
            int effects[] = { GF_NETHER, GF_NEXUS, GF_DISENCHANT, -1 };
            return _get_effect(effects);
        }
        return GF_NETHER;
    case DRAGON_LAW: 
    {
        int effects[] = { GF_SOUND, GF_SHARDS, -1 };
        return _get_effect(effects);
    }
    case DRAGON_CHAOS: 
    {
        int effects[] = { GF_CHAOS, GF_DISENCHANT, -1 };
        return _get_effect(effects);
    }
    case DRAGON_ETHEREAL: 
        if (p_ptr->lev < 40)
        {
            int effects[] = { GF_LITE, GF_DARK, -1 };
            return _get_effect(effects);
        }
        else
        {
            int effects[] = { GF_LITE, GF_DARK, GF_CONFUSION, -1 };
            return _get_effect(effects);
        }
    case DRAGON_CRYSTAL: return GF_SHARDS;
    case DRAGON_BALANCE:
    {
        int effects[] = { GF_SOUND, GF_SHARDS, GF_CHAOS, GF_DISENCHANT, -1 };
        return _get_effect(effects);
    }
    }
    return 0;
}
static int _breath_amount(void)
{
    int l = p_ptr->lev;
    int amt = 0;
    dragon_realm_ptr realm = _get_realm();

    switch (p_ptr->psubrace)
    {
    case DRAGON_RED:
    case DRAGON_WHITE:
    case DRAGON_BLUE:
    case DRAGON_BLACK:
    case DRAGON_GREEN:
        amt = MIN(600, p_ptr->chp * (25 + l*l*l/2500) / 100);
        break;

    case DRAGON_LAW:
    case DRAGON_CHAOS:
    case DRAGON_CRYSTAL:
    case DRAGON_BRONZE:
    case DRAGON_GOLD:
        amt = MIN(450, p_ptr->chp * (20 + l*l*l*30/125000) / 100);
        break;

    case DRAGON_BALANCE:
        amt = MIN(400, p_ptr->chp * (20 + l*l*l*25/125000) / 100);
        break;

    case DRAGON_NETHER:
    case DRAGON_ETHEREAL:
        amt = MIN(375, p_ptr->chp * (20 + l*l*l*15/125000) / 100);
        break;

    case DRAGON_STEEL:
        return 0;
    }
    amt = MAX(1, amt * realm->breath / 100);
    return amt;
}

static int _breath_cost(void)
{
    int l = p_ptr->lev;
    int cost = l/2 + l*l*15/2500;
    if (p_ptr->dragon_realm == DRAGON_REALM_BREATH && p_ptr->lev >= 40)
        cost = cost * 3 / 4;
    return MAX(cost, 1);
}

static cptr _breath_desc(void)
{
    switch (p_ptr->psubrace)
    {
    case DRAGON_RED: return "fire";
    case DRAGON_WHITE: return "cold";
    case DRAGON_BLUE: return "lightning";
    case DRAGON_BLACK: return "acid";
    case DRAGON_GREEN: return "poison";
    case DRAGON_BRONZE: return "confusion";
    case DRAGON_GOLD: return "sound";
    case DRAGON_NETHER: 
        if (p_ptr->lev >= 40) return "nether, nexus or disenchantment";
        return "nether";
    case DRAGON_LAW: return "sound or shards";
    case DRAGON_CHAOS: return "chaos or disenchantment";
    case DRAGON_ETHEREAL: return "light, dark or confusion";
    case DRAGON_CRYSTAL: return "shards";
    case DRAGON_BALANCE: return "sound, shards, chaos or disenchantment";
    }
    return 0;
}

static void _breathe_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe");
        break;
    case SPELL_DESC:
        var_set_string(res, format("Breathes %s at your opponent.", _breath_desc()));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _breath_amount()));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _breath_cost());
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_aim_dir(&dir))
        {
            int e = _breath_effect();
            int dam = _breath_amount();
            var_set_bool(res, FALSE);
            if (e < 0) return;
            msg_format("You breathe %s", gf_name(e));
            if (p_ptr->lev < 20)
                fire_bolt(e, dir, dam);
            else if (p_ptr->lev < 30)
                fire_beam(e, dir, dam);
            else
                fire_ball(e, dir, dam, -1 - (p_ptr->lev / 20));
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

/**********************************************************************
 * Dragon Melee
 **********************************************************************/
static int _attack_level(void)
{
    int l = p_ptr->lev * 2;
    switch (p_ptr->psubrace)
    {
    case DRAGON_STEEL:
        l = MAX(1, l * 130 / 100);
        break;

    case DRAGON_RED:
    case DRAGON_WHITE:
        l = MAX(1, l * 105 / 100);
        break;

    case DRAGON_BLACK:
    case DRAGON_GREEN:
        break;

    case DRAGON_BLUE:
        l = MAX(1, l * 95 / 100);
        break;

    case DRAGON_ETHEREAL:
    case DRAGON_CRYSTAL:
    case DRAGON_BRONZE:
    case DRAGON_GOLD:
        l = MAX(1, l * 90 / 100);
        break;

    case DRAGON_LAW:
    case DRAGON_CHAOS:
    case DRAGON_NETHER:
    case DRAGON_BALANCE:
        l = MAX(1, l * 85 / 100);
        break;
    }

    l = MAX(1, l * _get_realm()->attack / 100);
    return l;
}

/*
static int _bite_effect(void)
{
    switch (p_ptr->psubrace)
    {
    case DRAGON_RED: return GF_FIRE;
    case DRAGON_WHITE: return GF_COLD;
    case DRAGON_BLUE: return GF_ELEC;
    case DRAGON_BLACK: return GF_ACID;
    case DRAGON_GREEN: return GF_POIS;
    case DRAGON_BRONZE: return GF_OLD_CONF;
    }
    return 0;
}*/

static void _calc_innate_attacks(void)
{
    int l = _attack_level();
    int to_d = 0;
    int to_h = l/10 + l*l/500 + l*l*l/25000;

    /* Claws */
    {
        innate_attack_t    a = {0};

        a.dd = 1 + l / 30;
        a.ds = 3 + l / 10;
        a.to_h += to_h;
        a.to_d += to_d;

        a.weight = 100 + l;
        calc_innate_blows(&a, 400);
        a.msg = "You claw %s.";
        a.name = "Claw";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
    /* Bite */
    {
        innate_attack_t    a = {0};

        a.dd = 1 + l / 18;
        a.ds = 4 + l / 6;
        a.to_h += to_h;
        a.to_d += to_d;

        a.weight = 200 + 2 * l;

        /*TODO: if (p_ptr->lev >= 30)
            a.effect[1] = _bite_effect();*/

        if (p_ptr->lev >= 40)
            calc_innate_blows(&a, 200);
        else if (p_ptr->lev >= 35)
            calc_innate_blows(&a, 150);
        else
            a.blows = 100;
        a.msg = "You bite %s.";
        a.name = "Bite";
        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}

/**********************************************************************
 * Dragon Realms
 **********************************************************************/
/* Breath */
static void _bolt_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Bolt");
        break;
    case SPELL_DESC:
        var_set_string(res, format("Breathes a bolt of %s at your opponent. This is quicker, though less deadly, as you become more powerful.", _breath_desc()));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, MAX(1, _breath_amount()/2)));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, MAX(1, _breath_cost()/2));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_aim_dir(&dir))
        {
            int e = _breath_effect();
            int dam = MAX(1, _breath_amount()/2);
            var_set_bool(res, FALSE);
            if (e < 0) return;
            msg_format("You breathe %s", gf_name(e));
            fire_bolt(e, dir, dam);
            var_set_bool(res, TRUE);
        }
        break;
    }
    case SPELL_ENERGY:
        var_set_int(res, 101 - p_ptr->lev);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _beam_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Beam");
        break;
    case SPELL_DESC:
        var_set_string(res, format("Breathes a beam of %s at your opponent. This is quicker as you become more powerful.", _breath_desc()));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _breath_amount()));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _breath_cost());
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_aim_dir(&dir))
        {
            int e = _breath_effect();
            int dam = _breath_amount();
            var_set_bool(res, FALSE);
            if (e < 0) return;
            msg_format("You breathe %s", gf_name(e));
            fire_beam(e, dir, dam);
            var_set_bool(res, TRUE);
        }
        break;
    }
    case SPELL_ENERGY:
        var_set_int(res, 110 - p_ptr->lev);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _cone_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Cone");
        break;
    case SPELL_DESC:
        var_set_string(res, format("Breathes a cone of %s at your opponent. This is quicker as you become more powerful.", _breath_desc()));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _breath_amount()));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _breath_cost());
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_aim_dir(&dir))
        {
            int e = _breath_effect();
            int dam = _breath_amount();
            var_set_bool(res, FALSE);
            if (e < 0) return;
            msg_format("You breathe %s", gf_name(e));
            fire_ball(e, dir, dam, -2);
            var_set_bool(res, TRUE);
        }
        break;
    }
    case SPELL_ENERGY:
        var_set_int(res, 120 - p_ptr->lev);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _split_beam_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Split Beam");
        break;
    case SPELL_DESC:
        var_set_string(res, format("You breathe a beam of %s a two chosen targets, albeit with reduced damage.", _breath_desc()));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, MAX(1, _breath_amount()/2)));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _breath_cost());
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_aim_dir(&dir))
        {
            int e = _breath_effect();
            int dam = MAX(1, _breath_amount()/2);
            var_set_bool(res, FALSE);
            if (e < 0) return;
            msg_format("You breathe %s", gf_name(e));
            fire_beam(e, dir, dam);
            
            command_dir = 0; /* Code is buggy asking for a direction 2x in a single player action! */
            target_who = 0;  /* TODO: Repeat command is busted ... */
            if (get_aim_dir(&dir))
                fire_beam(e, dir, dam);

            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _retreating_breath_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Retreating Breath");
        break;
    case SPELL_DESC:
        var_set_string(res, format("Breathes %s at your opponent and then take a movement.", _breath_desc()));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _breath_amount()));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _breath_cost());
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_aim_dir(&dir))
        {
            int e = _breath_effect();
            int dam = _breath_amount();
            var_set_bool(res, FALSE);
            if (e < 0) return;
            msg_format("You breathe %s", gf_name(e));
            fire_ball(e, dir, dam, -2);

            command_dir = 0; /* Code is buggy asking for a direction 2x in a single player action! */
            target_who = 0;  /* TODO: Repeat command is busted ... */

            if (get_rep_dir2(&dir) && dir != 5)
            {
                int y, x;
                y = py + ddy[dir];
                x = px + ddx[dir];
                if (player_can_enter(cave[y][x].feat, 0) && !is_trap(cave[y][x].feat) && !cave[y][x].m_idx)
                    move_player_effect(y, x, MPE_FORGET_FLOW | MPE_HANDLE_STUFF | MPE_DONT_PICKUP);
            }

            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _deadly_breath_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Deadly Breath");
        break;
    case SPELL_DESC:
        var_set_string(res, format("Breathes %s powerfully at your opponent.", _breath_desc()));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _breath_amount() * 125 / 100));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _breath_cost() * 125 / 100);
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_aim_dir(&dir))
        {
            int e = _breath_effect();
            int dam = _breath_amount() * 125 / 100;
            var_set_bool(res, FALSE);
            if (e < 0) return;
            msg_format("You breathe %s", gf_name(e));
            fire_ball(e, dir, dam, -3);
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _star_ball_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Star Ball");
        break;
    case SPELL_DESC:
        var_set_string(res, "Unleash your breath uncontrollably and at random, though with devastating effect.");
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _breath_cost());
        break;
    case SPELL_CAST:
    {
        int num = damroll(5, 3);
        int dam = MAX(1, _breath_amount()/3);
        int e = _breath_effect();
        int y = py, x = px, i;
        int attempts;

        var_set_bool(res, FALSE);
        if (e < 0) return;

        for (i = 0; i < num; i++)
        {
            attempts = 1000;
            while (attempts--)
            {
                scatter(&y, &x, py, px, 4, 0);
                if (!cave_have_flag_bold(y, x, FF_PROJECT)) continue;
                if (!player_bold(y, x)) break;
            }
            project(0, 3, y, x, dam, e,
                (PROJECT_THRU | PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL), -1);
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

/* Craft */
static void _detect_magic_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Detect Magic");
        break;
    case SPELL_DESC:
        var_set_string(res, "Locate nearby magical objects.");
        break;
    case SPELL_CAST:
        detect_objects_magic(DETECT_RAD_DEFAULT);    
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _reforging_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Reforging");
        break;
    case SPELL_DESC:
        var_set_string(res, "Reforge a chosen artifact.");
        break;
    case SPELL_CAST:
    {
        int src_idx, dest_idx, cost;
        char o_name[MAX_NLEN];
        char buf[255];
        object_type *src, *dest;
        int power = p_ptr->lev * 5 / 2;
        int src_max_power = power * power * 10;
        int dest_max_power = 0;

        var_set_bool(res, FALSE);
        item_tester_hook = object_is_artifact;
        sprintf(buf, "Use what artifact for reforging (Max Power = %d)? ", src_max_power);
        if (!get_item(&src_idx, buf, "You have no artifacts to reforge.", USE_INVEN | SHOW_VALUE))
            return;

        src = &inventory[src_idx];
        if (!object_is_artifact(src)) /* paranoia */
        {
            msg_print("You must choose an artifact for reforging.");
            return;
        }
        if (object_value_real(src) > src_max_power)
        {
            msg_print("You are not powerful enough to reforge that item.");
            return;
        }

        cost = object_value_real(src);
    
        dest_max_power = cost / 2;
        if (dest_max_power < 1000) /* Reforging won't try to power match weak stuff ... */
            dest_max_power = 1000;

        object_desc(o_name, src, OD_NAME_ONLY);    
        if (!get_check(format("Really use %s? (It will be destroyed!) ", o_name))) 
            return;

        sprintf(buf, "Reforge which object (Max Power = %d)? ", dest_max_power);
        item_tester_hook = item_tester_hook_nameless_weapon_armour;
        if (!get_item(&dest_idx, buf, "You have nothing to reforge.", USE_EQUIP | USE_INVEN | SHOW_VALUE))
            return;

        dest = &inventory[dest_idx];

        if (dest->number > 1)
        {
            msg_print("Don't be greedy! You may only reforge a single object.");
            return;
        }

        if (object_is_artifact(dest))
        {
            msg_print("This item is already an artifact!");
            return;
        }

        if (object_is_ego(dest))
        {
            msg_print("This item is already an ego item!");
            return;
        }

        if (!equip_first_slot(dest))
        {
            msg_print("You may only create items you can actually use.");
            return;
        }

        if (object_value_real(dest) > dest_max_power)
        {
            msg_print("This item is too powerful for the source artifact you have chosen.");
            return;
        }

        if (!reforge_artifact(src, dest, power))
        {
            msg_print("The reforging failed!");
            return;
        }

        inven_item_increase(src_idx, -1);
        inven_item_describe(src_idx);

        object_aware(dest);
        object_known(dest);
        dest->ident |= IDENT_MENTAL;

        p_ptr->update |= PU_BONUS;
        p_ptr->window |= (PW_INVEN | PW_EQUIP);
        handle_stuff();

        screen_object(dest, 0);

        inven_item_optimize(src_idx);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static dragon_realm_t _realms[DRAGON_REALM_MAX] = {
    { "None", 
        "",
    /*  S   I   W   D   C   C    Dsrm Dvce Save Stlh Srch Prcp Thn Thb  Life  Exp Attack Breath*/
      { 0,  0,  0,  0,  0,  0}, {   0,   0,   0,   0,   0,   0,  0,  0}, 100, 100,   100,   100, A_NONE},

    { "Lore", 
        "Dragons specializing in lore are seekers of knowledge. They are the most "
        "intelligent of dragonkind and use their vast intellects to drive their magic. "
        "Armed with a vast array of detection and knowledge spells, dragons of lore "
        "seek power through knowledge. They eventually gain powers of telepathy and "
        "automatic object identification.",
    /*  S   I   W   D   C   C    Dsrm Dvce Save Stlh Srch Prcp Thn Thb  Life  Exp Attack Breath*/
      {-1, +3,  0, -1, -1,  0}, {   3,   8,   2,   0,   5,   5, -8,  0},  98, 100,    95,    95, A_INT},

    { "Breath", 
        "Dragon breath is the stuff of legends, and this realm seeks to enhance this most "
        "powerful attribute of dragonkind. With this speciality, you will be able to shape "
        "and control your breaths to maximize deadliness for a given situation. In addition, "
        "dragons of this realm may choose their breath types if applicable, and breathing "
        "becomes less costly as they mature. This focus requires great fortitude to master "
        "and somewhat dimishes the dragon's defenses and melee.",
    /*  S   I   W   D   C   C    Dsrm Dvce Save Stlh Srch Prcp Thn Thb  Life  Exp Attack Breath*/
      { 0, -1, -1,  0, +3, +1}, {   0,   0,   3,  -1,   0,   0,  0,  0}, 103, 105,    90,   115, A_CON},

    { "Attack", 
        "",
    /*  S   I   W   D   C   C    Dsrm Dvce Save Stlh Srch Prcp Thn Thb  Life  Exp Attack Breath*/
      {+3, -2, -2, +1, -1,  0}, {  -5,  -5,  -3,  -1,  -2,  -2, 15,  0},  97, 105,   115,    90, A_STR},

    { "Craft", 
        "The most powerful magical items have long been believed forged by dragonflame. The "
        "craft dragon gains powers of enchantment and may even reforge artifacts into the objects "
        "of their choosing!",
    /*  S   I   W   D   C   C    Dsrm Dvce Save Stlh Srch Prcp Thn Thb  Life  Exp Attack Breath*/
      {-1, -1, +3, -1, -1, -1}, {   3,   5,   0,   0,   0,   0, -5,  0}, 100, 107,    95,    95, A_WIS},

    { "Armor", 
        "",
    /*  S   I   W   D   C   C    Dsrm Dvce Save Stlh Srch Prcp Thn Thb  Life  Exp Attack Breath*/
      {-1, -1, -1, +3, +1, +1}, {  -2,  -3,   7,   1,   0,   0,-10,  0}, 102, 105,    95,    95, A_DEX},

    { "Recovery", 
        "",
    /*  S   I   W   D   C   C    Dsrm Dvce Save Stlh Srch Prcp Thn Thb  Life  Exp Attack Breath*/
      {-1,  0,  0, -1, +3,  0}, {   0,   0,   0,   0,   0,   0,  0,  0}, 100, 107,   100,   100, A_CON},

    { "Domination", 
        "",
    /*  S   I   W   D   C   C    Dsrm Dvce Save Stlh Srch Prcp Thn Thb  Life  Exp Attack Breath*/
      {-1, -1, -1, -1, -1, +3}, {  -2,  -3,  -2,   0,   0,   0, -7,  0},  95, 105,    95,    90, A_CHR},

    { "Crusade", 
        "",
    /*  S   I   W   D   C   C    Dsrm Dvce Save Stlh Srch Prcp Thn Thb  Life  Exp Attack Breath*/
      {+1, -1, -1, +1, -1, +2}, {  -5,   0,  -2,   0,  -2,  -2,  7,  0},  95, 107,   105,   105, A_CHR},
};

dragon_realm_ptr dragon_get_realm(int which)
{
    assert(0 <= which && which < DRAGON_REALM_MAX);
    return &_realms[which];
}

dragon_realm_ptr _get_realm(void)
{
    return dragon_get_realm(p_ptr->dragon_realm);
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "dragon spell";
        me.weight = 750;
        init = TRUE;
    }
    me.which_stat = _get_realm()->spell_stat; /* Careful: Birthing may invoke this multiple times with different realms */
    return &me;
}

static spell_info _lore_spells[] = {
    {  1,  1, 30, detect_traps_spell },
    {  3,  2, 30, detect_treasure_spell },
    {  5,  3, 40, detect_monsters_spell },
    {  7,  5, 50, detect_objects_spell },
    { 12, 10, 60, identify_spell },
    { 15, 12, 60, sense_surroundings_spell },
    { 20, 15, 60, detection_spell },
    { 22, 17, 60, probing_spell },
    { 25, 20, 65, self_knowledge_spell },
    { 30, 25, 70, identify_fully_spell },
    { 40, 50, 90, clairvoyance_spell },
    { -1, -1, -1, NULL}
};

static spell_info _breath_spells[] = {
    {  1,  0, 30, _bolt_spell },
    { 10,  0, 30, _beam_spell },
    { 20,  0, 30, _cone_spell },
    { 25, 10, 50, _split_beam_spell },
    { 30, 15, 50, _retreating_breath_spell },
    { 40, 15, 60, _deadly_breath_spell },
    { 50, 50, 70, _star_ball_spell },
    { -1, -1, -1, NULL}
};

static spell_info _craft_spells[] = {
    {  1,  1, 30, _detect_magic_spell },
    {  5,  7, 60, minor_enchantment_spell },
    { 12, 10, 60, remove_curse_I_spell },
    { 17, 15, 60, identify_spell },
    { 30, 25, 70, enchantment_spell },
    { 32, 30, 70, recharging_spell },
    { 35, 90, 90, _reforging_spell },
    { -1, -1, -1, NULL}
};

int _realm_get_spells(spell_info* spells, int max)
{
    switch (p_ptr->dragon_realm)
    {
    case DRAGON_REALM_LORE:
        return get_spells_aux(spells, max, _lore_spells);
    case DRAGON_REALM_BREATH:
        return get_spells_aux(spells, max, _breath_spells);
    case DRAGON_REALM_CRAFT:
        return get_spells_aux(spells, max, _craft_spells);
    }
    return 0;
}

static void _realm_calc_bonuses(void)
{
    switch (p_ptr->dragon_realm)
    {
    case DRAGON_REALM_LORE:
        if (p_ptr->lev >= 35)
            p_ptr->telepathy = TRUE;
        if (p_ptr->lev >= 40)
            p_ptr->loremaster = TRUE;
        break;
    case DRAGON_REALM_BREATH:
        p_ptr->to_a -= p_ptr->lev/2;
        p_ptr->dis_to_a -= p_ptr->lev/2;
        break;
    }
}

static void _realm_get_flags(u32b flgs[TR_FLAG_SIZE]) 
{
    switch (p_ptr->dragon_realm)
    {
    case DRAGON_REALM_LORE:
        if (p_ptr->lev >= 35)
            add_flag(flgs, TR_TELEPATHY);
        break;
    }
}

/**********************************************************************
 * Dragon Bonuses (Common to all Types)
 **********************************************************************/
static void _dragon_calc_bonuses(void) 
{
    p_ptr->skill_dig += 100;
    p_ptr->levitation = TRUE;
    if (p_ptr->lev >= 20)
    {
        p_ptr->free_act = TRUE;
        p_ptr->see_inv = TRUE;
    }
    if (p_ptr->lev >= 30)
    {
        res_add(RES_CONF);
        res_add(RES_FEAR);
    }
    _realm_calc_bonuses();
}

static void _dragon_get_flags(u32b flgs[TR_FLAG_SIZE]) 
{
    add_flag(flgs, TR_LEVITATION);
    if (p_ptr->lev >= 20)
    {
        add_flag(flgs, TR_FREE_ACT);
        add_flag(flgs, TR_SEE_INVIS);
    }
    if (p_ptr->lev >= 30)
    {
        add_flag(flgs, TR_RES_CONF);
        add_flag(flgs, TR_RES_FEAR);
    }
    _realm_get_flags(flgs);
}

/**********************************************************************
 * Elemental Dragon (Red, White, Blue, Black, Green)
 *   Baby -> Young -> Mature -> Ancient -> Great Foo Wyrm
 **********************************************************************/
 typedef struct {
    int  r_idx[5];
    cptr r_name[5];
    int  which_res;
} _elemental_info_t;

static _elemental_info_t _elemental_info[5] = { /* relies on #define DRAGON_RED 0 ... */
    { {167, 563, 589, 644, 756},
      {"Baby Red Dragon", "Young Red Dragon", "Mature Red Dragon", "Ancient Red Dragon", "Great Hell Wyrm"},
      RES_FIRE },
    { {164, 460, 549, 617, 741},
      {"Baby White Dragon", "Young White Dragon", "Mature White Dragon", "Ancient White Dragon", "Great Ice Wyrm"},
      RES_COLD},
    { {163, 459, 560, 601, 728},
      {"Baby Blue Dragon", "Young Blue Dragon", "Mature Blue Dragon", "Ancient Blue Dragon", "Great Storm Wyrm"},
      RES_ELEC},
    { {166, 546, 592, 624, 1066},
      {"Baby Black Dragon", "Young Black Dragon", "Mature Black Dragon", "Ancient Black Dragon", "Great Bile Wyrm"},
      RES_ACID},
    { {165, 461, 561, 618, 890},
      {"Baby Green Dragon", "Young Green Dragon", "Mature Green Dragon", "Ancient Green Dragon", "Great Venom Wyrm"},
      RES_POIS},
};

static power_info _elemental_powers[] = {
    { A_CON, {  1,  0, 30, _breathe_spell}},
    {    -1, { -1, -1, -1, NULL} }
};
static int _elemental_get_powers(spell_info* spells, int max) {
    return get_powers_aux(spells, max, _elemental_powers);
}
static void _elemental_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = l/2 + l*l/100 + l*l*l/5000;
    int ac = 15 + (l/10)*5;
    int res = _elemental_info[p_ptr->psubrace].which_res;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(res);
    
    if (p_ptr->lev >= 30)
    {
        p_ptr->pspeed += 3;
        res_add(res);
    }
    if (p_ptr->lev >= 40)
    {
        p_ptr->pspeed += 2;
        res_add_immune(res);
        res_add(RES_BLIND);
        switch (res)
        {
        case RES_FIRE: p_ptr->sh_fire = TRUE; break;
        case RES_COLD: p_ptr->sh_cold = TRUE; break;
        case RES_ELEC: p_ptr->sh_elec = TRUE; break;
        }
    }
    _dragon_calc_bonuses();
}
static void _elemental_get_flags(u32b flgs[TR_FLAG_SIZE]) {
    int res = _elemental_info[p_ptr->psubrace].which_res;
    add_flag(flgs, res_get_object_flag(res));
    if (p_ptr->lev >= 30)
    {
        add_flag(flgs, TR_SPEED);
    }
    if (p_ptr->lev >= 40)
    {
        add_flag(flgs, TR_RES_BLIND);
        switch (res)
        {
        case RES_FIRE: add_flag(flgs, TR_SH_FIRE); break;
        case RES_COLD: add_flag(flgs, TR_SH_COLD); break;
        case RES_ELEC: add_flag(flgs, TR_SH_ELEC); break;
        }
    }
    _dragon_get_flags(flgs);
}
static void _elemental_get_immunities(u32b flgs[TR_FLAG_SIZE]) {
    int res = _elemental_info[p_ptr->psubrace].which_res;
    if (p_ptr->lev >= 40)
        add_flag(flgs, res_get_object_flag(res));
}
static void _elemental_birth(void) { 
    p_ptr->current_r_idx = _elemental_info[p_ptr->psubrace].r_idx[0]; 
    _dragon_birth();
}
static void _elemental_gain_level(int new_level) {
    if (p_ptr->current_r_idx == _elemental_info[p_ptr->psubrace].r_idx[0] && new_level >= 10)
    {
        p_ptr->current_r_idx = _elemental_info[p_ptr->psubrace].r_idx[1];
        msg_format("You have evolved into a %s.", _elemental_info[p_ptr->psubrace].r_name[1]);
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == _elemental_info[p_ptr->psubrace].r_idx[1] && new_level >= 20)
    {
        p_ptr->current_r_idx = _elemental_info[p_ptr->psubrace].r_idx[2];
        msg_format("You have evolved into a %s.", _elemental_info[p_ptr->psubrace].r_name[2]);
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == _elemental_info[p_ptr->psubrace].r_idx[2] && new_level >= 30)
    {
        p_ptr->current_r_idx = _elemental_info[p_ptr->psubrace].r_idx[3];
        msg_format("You have evolved into an %s.", _elemental_info[p_ptr->psubrace].r_name[3]);
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == _elemental_info[p_ptr->psubrace].r_idx[3] && new_level >= 40)
    {
        p_ptr->current_r_idx = _elemental_info[p_ptr->psubrace].r_idx[4];
        msg_format("You have evolved into a %s.", _elemental_info[p_ptr->psubrace].r_name[4]);
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_elemental_get_race_t(int subrace)
{
    static race_t me = {0};
    static bool   init = FALSE;
    int           rank = 0;

    if (p_ptr->lev >= 10) rank++;
    if (p_ptr->lev >= 20) rank++;
    if (p_ptr->lev >= 30) rank++;
    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  35,  38,   2,  25,  26,  70,  30};
    skills_t xs = {  8,   9,  10,   0,   0,   0,  20,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 250;

        me.birth = _elemental_birth;
        me.get_powers = _elemental_get_powers;
        me.calc_bonuses = _elemental_calc_bonuses;
        me.get_flags = _elemental_get_flags;
        me.get_immunities = _elemental_get_immunities;
        me.gain_level = _elemental_gain_level;
        init = TRUE;
    }

    me.subname = _elemental_info[subrace].r_name[rank];
    me.stats[A_STR] =  1 + rank;
    me.stats[A_INT] = -1 + rank;
    me.stats[A_WIS] = -2 + rank;
    me.stats[A_DEX] = -2 + rank;
    me.stats[A_CON] =  0 + rank;
    me.stats[A_CHR] = -1 + rank;
    me.life = 100 + 5*rank;

    return &me;
}

/**********************************************************************
 * Nether: Shadow Drake -> Death Drake -> Spectral Wyrm
 **********************************************************************/
static power_info _nether_powers[] = {
    { A_CON, {  1,  0, 30, _breathe_spell}},
    { A_DEX, { 45,  5, 30, phase_door_spell}}, 
    { A_CHR, { 45, 90, 90, summon_hi_dragon_spell}}, 
    {    -1, { -1, -1, -1, NULL} }
};
static int _nether_get_powers(spell_info* spells, int max) {
    return get_powers_aux(spells, max, _nether_powers);
}
static void _nether_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = l/2 + l*l/100 + l*l*l/5000;
    int ac = 15 + (l/10)*2;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(RES_NETHER);
    
    if (p_ptr->lev >= 30)
    {
        p_ptr->pspeed += 3;
        res_add(RES_COLD);
        res_add(RES_TELEPORT);
        p_ptr->pass_wall = TRUE;
        p_ptr->no_passwall_dam = TRUE;
    }
    if (p_ptr->lev >= 45)
    {
        p_ptr->align -= 200;
        p_ptr->pspeed += 2;
        p_ptr->sh_cold = TRUE;
        res_add(RES_POIS);
        res_add_immune(RES_NETHER);
        res_add(RES_NEXUS);
        res_add(RES_DISEN);
        res_add(RES_TELEPORT);
    }
    _dragon_calc_bonuses();
}
static void _nether_get_flags(u32b flgs[TR_FLAG_SIZE]) {
    add_flag(flgs, TR_RES_NETHER);
    if (p_ptr->lev >= 30)
    {
        add_flag(flgs, TR_SPEED);
        add_flag(flgs, TR_RES_COLD);
    }
    if (p_ptr->lev >= 45)
    {
        add_flag(flgs, TR_SH_COLD);
        add_flag(flgs, TR_RES_POIS);
        add_flag(flgs, TR_RES_NEXUS);
        add_flag(flgs, TR_RES_DISEN);
    }
    _dragon_get_flags(flgs);
}
static void _nether_get_immunities(u32b flgs[TR_FLAG_SIZE]) {
    if (p_ptr->lev >= 45)
        add_flag(flgs, TR_RES_NETHER);
}
static void _nether_birth(void) { 
    p_ptr->current_r_idx = MON_SHADOW_DRAKE; 
    _dragon_birth();
}

static void _nether_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_SHADOW_DRAKE && new_level >= 30)
    {
        p_ptr->current_r_idx = MON_DEATH_DRAKE;
        msg_print("You have evolved into a Death Drake.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_DEATH_DRAKE && new_level >= 45)
    {
        p_ptr->current_r_idx = MON_SPECTRAL_WYRM;
        msg_print("You have evolved into a Spectral Wyrm.");
        p_ptr->redraw |= PR_MAP;
    }
}

static race_t *_nether_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[3] =  {"Shadow Drake", "Death Drake", "Spectral Wyrm"};    
    int           rank = 0;

    if (p_ptr->lev >= 30) rank++;
    if (p_ptr->lev >= 45) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  35,  38,   4,  25,  26,  50,  30};
    skills_t xs = {  8,  10,  11,   0,   0,   0,  15,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 350;

        me.birth = _nether_birth;
        me.get_powers = _nether_get_powers;
        me.calc_bonuses = _nether_calc_bonuses;
        me.get_flags = _nether_get_flags;
        me.get_immunities = _nether_get_immunities;
        me.gain_level = _nether_gain_level;
        init = TRUE;
    }

    me.subname = titles[rank];
    me.stats[A_STR] =  0 + 2*rank;
    me.stats[A_INT] = -1 + 2*rank;
    me.stats[A_WIS] = -2 + rank;
    me.stats[A_DEX] = -2 + rank;
    me.stats[A_CON] = -1 + rank;
    me.stats[A_CHR] = -1 + 3*rank;
    me.life = 90 + 5*rank;

    return &me;
}

/**********************************************************************
 * Law: Law Drake -> Great Wyrm of Law
 **********************************************************************/
static power_info _law_powers[] = {
    { A_CON, {  1,  0, 30, _breathe_spell}},
    { A_CHR, { 40, 70, 65, summon_dragon_spell}}, 
    {    -1, { -1, -1, -1, NULL} }
};
static int _law_get_powers(spell_info* spells, int max) {
    return get_powers_aux(spells, max, _law_powers);
}
static void _law_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = l/2 + l*l/100 + l*l*l/5000;
    int ac = 15 + (l/10)*2;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(RES_SOUND);
    res_add(RES_SHARDS);

    if (p_ptr->lev >= 40)
    {
        p_ptr->align += 200;
        p_ptr->pspeed += 5;
        res_add(RES_SOUND);
        res_add(RES_SHARDS);
    }

    _dragon_calc_bonuses();
}
static void _law_get_flags(u32b flgs[TR_FLAG_SIZE]) {
    add_flag(flgs, TR_RES_SOUND);
    add_flag(flgs, TR_RES_SHARDS);
    if (p_ptr->lev >= 40)
    {
        add_flag(flgs, TR_SPEED);
    }
    _dragon_get_flags(flgs);
}
static void _law_birth(void) { 
    p_ptr->current_r_idx = MON_LAW_DRAKE; 
    _dragon_birth();
}
static void _law_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_LAW_DRAKE && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_GREAT_WYRM_OF_LAW;
        msg_print("You have evolved into a Great Wyrm of Law.");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_law_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[2] =  {"Law Drake", "Great Wyrm of Law"};    
    int           rank = 0;

    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  40,  40,   2,  25,  26,  55,  30};
    skills_t xs = {  8,  11,  11,   0,   0,   0,  15,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 300;

        me.birth = _law_birth;
        me.get_powers = _law_get_powers;
        me.calc_bonuses = _law_calc_bonuses;
        me.get_flags = _law_get_flags;
        me.gain_level = _law_gain_level;
        init = TRUE;
    }

    me.subname = titles[rank];
    me.stats[A_STR] =  0 + 5*rank;
    me.stats[A_INT] = -1 + 5*rank;
    me.stats[A_WIS] = -2 + 2*rank;
    me.stats[A_DEX] = -2 + 3*rank;
    me.stats[A_CON] = -1 + 4*rank;
    me.stats[A_CHR] = -1 + 5*rank;
    me.life = 100 + 10*rank;

    return &me;
}

/**********************************************************************
 * Chaos: Chaos Drake -> Great Wyrm of Chaos
 **********************************************************************/
static power_info _chaos_powers[] = {
    { A_CON, {  1,  0, 30, _breathe_spell}},
    { A_CHR, { 40, 70, 65, summon_dragon_spell}}, 
    {    -1, { -1, -1, -1, NULL} }
};
static int _chaos_get_powers(spell_info* spells, int max) {
    return get_powers_aux(spells, max, _chaos_powers);
}
static void _chaos_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = l/2 + l*l/100 + l*l*l/5000;
    int ac = 15 + (l/10)*2;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(RES_CHAOS);
    res_add(RES_DISEN);
    
    if (p_ptr->lev >= 40)
    {
        p_ptr->align -= 200;
        p_ptr->pspeed += 5;
        res_add(RES_CHAOS);
        res_add(RES_DISEN);
    }

    _dragon_calc_bonuses();
}
static void _chaos_get_flags(u32b flgs[TR_FLAG_SIZE]) {
    add_flag(flgs, TR_RES_CHAOS);
    add_flag(flgs, TR_RES_DISEN);
    if (p_ptr->lev >= 40)
    {
        add_flag(flgs, TR_SPEED);
    }
    _dragon_get_flags(flgs);
}
static void _chaos_birth(void) { 
    p_ptr->current_r_idx = MON_CHAOS_DRAKE; 
    _dragon_birth();
}
static void _chaos_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_CHAOS_DRAKE && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_GREAT_WYRM_OF_CHAOS;
        msg_print("You have evolved into a Great Wyrm of Chaos.");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_chaos_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[2] =  {"Chaos Drake", "Great Wyrm of Chaos"};    
    int           rank = 0;

    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  40,  40,   2,  25,  26,  55,  30};
    skills_t xs = {  8,  11,  11,   0,   0,   0,  15,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 300;

        me.birth = _chaos_birth;
        me.get_powers = _chaos_get_powers;
        me.calc_bonuses = _chaos_calc_bonuses;
        me.get_flags = _chaos_get_flags;
        me.gain_level = _chaos_gain_level;
        init = TRUE;
    }

    me.subname = titles[rank];
    me.stats[A_STR] =  0 + 5*rank;
    me.stats[A_INT] = -1 + 5*rank;
    me.stats[A_WIS] = -2 + 2*rank;
    me.stats[A_DEX] = -2 + 3*rank;
    me.stats[A_CON] = -1 + 4*rank;
    me.stats[A_CHR] = -1 + 5*rank;
    me.life = 100 + 10*rank;

    return &me;
}

/**********************************************************************
 * Balance: Balance Drake -> Great Wyrm of Balance
 **********************************************************************/
static power_info _balance_powers[] = {
    { A_CON, {  1,  0, 30, _breathe_spell}},
    { A_CHR, { 40, 70, 65, summon_dragon_spell}}, 
    { A_CHR, { 50, 90, 80, summon_hi_dragon_spell}}, 
    {    -1, { -1, -1, -1, NULL} }
};
static int _balance_get_powers(spell_info* spells, int max) {
    return get_powers_aux(spells, max, _balance_powers);
}
static void _balance_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = l/2 + l*l/100 + l*l*l/5000;
    int ac = 10 + (l/10)*2;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(RES_SOUND);
    res_add(RES_SHARDS);
    res_add(RES_CHAOS);
    res_add(RES_DISEN);
    
    if (p_ptr->lev >= 40)
    {
        p_ptr->pspeed += 5;
    }
    _dragon_calc_bonuses();
}
static void _balance_get_flags(u32b flgs[TR_FLAG_SIZE]) {
    add_flag(flgs, TR_RES_SOUND);
    add_flag(flgs, TR_RES_SHARDS);
    add_flag(flgs, TR_RES_CHAOS);
    add_flag(flgs, TR_RES_DISEN);
    if (p_ptr->lev >= 40)
    {
        add_flag(flgs, TR_SPEED);
    }
    _dragon_get_flags(flgs);
}
static void _balance_birth(void) { 
    p_ptr->current_r_idx = MON_BALANCE_DRAKE; 
    _dragon_birth();
}
static void _balance_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_BALANCE_DRAKE && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_GREAT_WYRM_OF_BALANCE;
        msg_print("You have evolved into a Great Wyrm of Balance.");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_balance_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[2] =  {"Balance Drake", "Great Wyrm of Balance"};    
    int           rank = 0;

    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  35,  35,   2,  25,  26,  50,  30};
    skills_t xs = {  8,  10,  10,   0,   0,   0,  15,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 350;

        me.birth = _balance_birth;
        me.get_powers = _balance_get_powers;
        me.calc_bonuses = _balance_calc_bonuses;
        me.get_flags = _balance_get_flags;
        me.gain_level = _balance_gain_level;
        init = TRUE;
    }

    me.subname = titles[rank];
    me.stats[A_STR] =  0 + 4*rank;
    me.stats[A_INT] = -1 + 4*rank;
    me.stats[A_WIS] = -2 + 2*rank;
    me.stats[A_DEX] = -2 + 3*rank;
    me.stats[A_CON] = -1 + 3*rank;
    me.stats[A_CHR] = -1 + 5*rank;
    me.life = 95 + 10*rank;

    return &me;
}

/**********************************************************************
 * Ethereal: Pseudo Dragon -> Ethereal Drake -> Ethereal Dragon
 **********************************************************************/
static power_info _ethereal_powers[] = {
    { A_CON, {  1,  0, 30, _breathe_spell}},
    {    -1, { -1, -1, -1, NULL} }
};
static int _ethereal_get_powers(spell_info* spells, int max) {
    return get_powers_aux(spells, max, _ethereal_powers);
}
static void _ethereal_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = l/2 + l*l/100 + l*l*l/5000;
    int ac = 15 + (l/10)*2;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(RES_LITE);
    res_add(RES_DARK);
    
    if (p_ptr->lev >= 20)
    {
        p_ptr->pass_wall = TRUE;
        p_ptr->no_passwall_dam = TRUE;
    }
    if (p_ptr->lev >= 40)
    {
        p_ptr->pspeed += 5;
        res_add(RES_LITE);
        res_add(RES_DARK);
        res_add(RES_CONF);
    }
    _dragon_calc_bonuses();
}
static void _ethereal_get_flags(u32b flgs[TR_FLAG_SIZE]) {
    add_flag(flgs, TR_RES_LITE);
    add_flag(flgs, TR_RES_DARK);
    if (p_ptr->lev >= 40)
    {
        add_flag(flgs, TR_SPEED);
    }
    _dragon_get_flags(flgs);
}
static void _ethereal_birth(void) { 
    p_ptr->current_r_idx = MON_PSEUDO_DRAGON; 
    _dragon_birth();
}
static void _ethereal_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_PSEUDO_DRAGON && new_level >= 20)
    {
        p_ptr->current_r_idx = MON_ETHEREAL_DRAKE;
        msg_print("You have evolved into an Ethereal Drake.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_ETHEREAL_DRAKE && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_ETHEREAL_DRAGON;
        msg_print("You have evolved into an Ethereal Dragon.");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_ethereal_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[3] =  {"Pseudo Dragon", "Ethereal Drake", "Ethereal Dragon"};    
    int           rank = 0;

    if (p_ptr->lev >= 20) rank++;
    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  35,  37,   4,  25,  26,  52,  30};
    skills_t xs = {  8,  10,  11,   0,   0,   0,  15,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 250;

        me.birth = _ethereal_birth;
        me.get_powers = _ethereal_get_powers;
        me.calc_bonuses = _ethereal_calc_bonuses;
        me.get_flags = _ethereal_get_flags;
        me.gain_level = _ethereal_gain_level;
        init = TRUE;
    }

    me.subname = titles[rank];
    me.stats[A_STR] =  0 + 2*rank;
    me.stats[A_INT] = -1 + 2*rank;
    me.stats[A_WIS] = -2 + rank;
    me.stats[A_DEX] = -2 + 2*rank;
    me.stats[A_CON] = -1 + 2*rank;
    me.stats[A_CHR] = -1 + 2*rank;
    me.life = 95 + 7*rank;

    return &me;
}

/**********************************************************************
 * Crystal: Crystal Drake -> Great Crystal Drake
 **********************************************************************/
static power_info _crystal_powers[] = {
    { A_CON, {  1,  0, 30, _breathe_spell}},
    {    -1, { -1, -1, -1, NULL} }
};
static int _crystal_get_powers(spell_info* spells, int max) {
    return get_powers_aux(spells, max, _crystal_powers);
}
static void _crystal_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = l/2 + l*l/50 + l*l*l/2500;
    int ac = 15 + (l/10)*2;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(RES_COLD);
    res_add(RES_SHARDS);
    if (p_ptr->lev >= 10)
    {
        p_ptr->pspeed++;
    }    
    if (p_ptr->lev >= 20)
    {
        p_ptr->pspeed++;
    }
    if (p_ptr->lev >= 30)
    {
        p_ptr->pspeed++;
    }
    if (p_ptr->lev >= 40)
    {
        p_ptr->pspeed += 2;
        res_add(RES_SHARDS);
        p_ptr->reflect = TRUE;
    }
    _dragon_calc_bonuses();
}
static void _crystal_get_flags(u32b flgs[TR_FLAG_SIZE]) {
    add_flag(flgs, TR_RES_COLD);
    add_flag(flgs, TR_RES_SHARDS);
    if (p_ptr->lev >= 10)
    {
        add_flag(flgs, TR_SPEED);
    }
    if (p_ptr->lev >= 40)
    {
        add_flag(flgs, TR_REFLECT);
    }
    _dragon_get_flags(flgs);
}
static void _crystal_birth(void) { 
    p_ptr->current_r_idx = MON_CRYSTAL_DRAKE; 
    _dragon_birth();
}
static void _crystal_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_CRYSTAL_DRAKE && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_GREAT_CRYSTAL_DRAKE;
        msg_print("You have evolved into a Great Crystal Drake.");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_crystal_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[2] =  {"Crystal Drake", "Great Crystal Drake"};    
    int           rank = 0;

    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  35,  40,   1,  25,  26,  70,  30};
    skills_t xs = {  8,   7,  12,   0,   0,   0,  22,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 275;

        me.birth = _crystal_birth;
        me.get_powers = _crystal_get_powers;
        me.calc_bonuses = _crystal_calc_bonuses;
        me.get_flags = _crystal_get_flags;
        me.gain_level = _crystal_gain_level;
        init = TRUE;
    }

    me.subname = titles[rank];
    me.stats[A_STR] =  1 + 5*rank;
    me.stats[A_INT] = -1 + 5*rank;
    me.stats[A_WIS] = -2 + 2*rank;
    me.stats[A_DEX] =  0 + 3*rank;
    me.stats[A_CON] =  0 + 4*rank;
    me.stats[A_CHR] =  0 + 3*rank;
    me.life = 100 + 15*rank;

    return &me;
}

/**********************************************************************
 * Bronze: Young -> Mature -> Ancient
 **********************************************************************/
static power_info _bronze_powers[] = {
    { A_CON, {  1,  0, 30, _breathe_spell}},
    {    -1, { -1, -1, -1, NULL} }
};
static int _bronze_get_powers(spell_info* spells, int max) {
    return get_powers_aux(spells, max, _bronze_powers);
}
static void _bronze_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = l/2 + l*l/100 + l*l*l/5000;
    int ac = 15 + (l/10)*2;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(RES_CONF);
    
    if (p_ptr->lev >= 30)
    {
        p_ptr->pspeed += 3;
    }
    if (p_ptr->lev >= 40)
    {
        p_ptr->pspeed += 2;
    }
    _dragon_calc_bonuses();
}
static void _bronze_get_flags(u32b flgs[TR_FLAG_SIZE]) {
    add_flag(flgs, TR_RES_CONF);
    if (p_ptr->lev >= 30)
    {
        add_flag(flgs, TR_SPEED);
    }
    _dragon_get_flags(flgs);
}
static void _bronze_birth(void) { 
    p_ptr->current_r_idx = MON_YOUNG_BRONZE_DRAGON; 
    _dragon_birth();
}
static void _bronze_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_YOUNG_BRONZE_DRAGON && new_level >= 20)
    {
        p_ptr->current_r_idx = MON_MATURE_BRONZE_DRAGON;
        msg_print("You have evolved into a Mature Bronze Dragon.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_MATURE_BRONZE_DRAGON && new_level >= 30)
    {
        p_ptr->current_r_idx = MON_ANCIENT_BRONZE_DRAGON;
        msg_print("You have evolved into an Ancient Bronze Dragon.");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_bronze_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[3] =  {"Young Bronze Dragon", "Mature Bronze Dragon", "Ancient Bronze Dragon"};    
    int           rank = 0;

    if (p_ptr->lev >= 20) rank++;
    if (p_ptr->lev >= 30) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  35,  38,   3,  25,  26,  55,  30};
    skills_t xs = {  8,  10,  11,   0,   0,   0,  15,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 250;

        me.birth = _bronze_birth;
        me.get_powers = _bronze_get_powers;
        me.calc_bonuses = _bronze_calc_bonuses;
        me.get_flags = _bronze_get_flags;
        me.gain_level = _bronze_gain_level;
        init = TRUE;
    }

    me.subname = titles[rank];
    me.stats[A_STR] =  0 + 2*rank;
    me.stats[A_INT] = -1 + 2*rank;
    me.stats[A_WIS] = -2 + rank;
    me.stats[A_DEX] = -2 + 2*rank;
    me.stats[A_CON] = -1 + 2*rank;
    me.stats[A_CHR] = -1 + 2*rank;
    me.life = 100 + 5*rank;

    return &me;
}

/**********************************************************************
 * Gold: Young -> Mature -> Ancient
 **********************************************************************/
static power_info _gold_powers[] = {
    { A_CON, {  1,  0, 30, _breathe_spell}},
    {    -1, { -1, -1, -1, NULL} }
};
static int _gold_get_powers(spell_info* spells, int max) {
    return get_powers_aux(spells, max, _gold_powers);
}
static void _gold_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = l/2 + l*l/100 + l*l*l/5000;
    int ac = 15 + (l/10)*2;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(RES_SOUND);
    
    if (p_ptr->lev >= 30)
    {
        p_ptr->pspeed += 3;
    }
    if (p_ptr->lev >= 40)
    {
        res_add(RES_SOUND);
        p_ptr->pspeed += 2;
    }
    _dragon_calc_bonuses();
}
static void _gold_get_flags(u32b flgs[TR_FLAG_SIZE]) {
    add_flag(flgs, TR_RES_SOUND);
    if (p_ptr->lev >= 30)
    {
        add_flag(flgs, TR_SPEED);
    }
    _dragon_get_flags(flgs);
}
static void _gold_birth(void) { 
    p_ptr->current_r_idx = MON_YOUNG_GOLD_DRAGON; 
    _dragon_birth();
}
static void _gold_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_YOUNG_GOLD_DRAGON && new_level >= 20)
    {
        p_ptr->current_r_idx = MON_MATURE_GOLD_DRAGON;
        msg_print("You have evolved into a Mature Gold Dragon.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_MATURE_GOLD_DRAGON && new_level >= 30)
    {
        p_ptr->current_r_idx = MON_ANCIENT_GOLD_DRAGON;
        msg_print("You have evolved into an Ancient Gold Dragon.");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_gold_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[3] =  {"Young Gold Dragon", "Mature Gold Dragon", "Ancient Gold Dragon"};    
    int           rank = 0;

    if (p_ptr->lev >= 20) rank++;
    if (p_ptr->lev >= 30) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  35,  38,   2,  25,  26,  55,  30};
    skills_t xs = {  8,   9,  11,   0,   0,   0,  20,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 250;

        me.birth = _gold_birth;
        me.get_powers = _gold_get_powers;
        me.calc_bonuses = _gold_calc_bonuses;
        me.get_flags = _gold_get_flags;
        me.gain_level = _gold_gain_level;
        init = TRUE;
    }

    me.subname = titles[rank];
    me.stats[A_STR] =  0 + 2*rank;
    me.stats[A_INT] = -1 + 2*rank;
    me.stats[A_WIS] = -2 + rank;
    me.stats[A_DEX] = -2 + 2*rank;
    me.stats[A_CON] = -1 + 2*rank;
    me.stats[A_CHR] = -1 + 2*rank;
    me.life = 100 + 5*rank;

    return &me;
}

/**********************************************************************
 * Steel: Stone Dragon -> Steel Dragon
 **********************************************************************/
static void _steel_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = l + l*l/50 + l*l*l/2500;
    int ac = 15 + (l/10)*2;

    p_ptr->skill_dig += 100;
    
    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    if (p_ptr->lev < 40)
        res_add_vuln(RES_COLD);

    res_add(RES_FIRE);
    res_add(RES_ELEC);
    res_add(RES_POIS);
    p_ptr->no_cut = TRUE;
    
    if (p_ptr->lev >= 30)
    {
        p_ptr->no_stun = TRUE;
    }
    if (p_ptr->lev >= 40)
    {
        res_add(RES_SHARDS);
        p_ptr->pspeed += 2;
    }
    _dragon_calc_bonuses();
}
static void _steel_get_flags(u32b flgs[TR_FLAG_SIZE]) {
    add_flag(flgs, TR_RES_FIRE);
    add_flag(flgs, TR_RES_ELEC);
    add_flag(flgs, TR_RES_POIS);
    if (p_ptr->lev >= 40)
    {
        add_flag(flgs, TR_RES_SHARDS);
        add_flag(flgs, TR_SPEED);
    }
    _dragon_get_flags(flgs);
}
static void _steel_get_vulnerabilities(u32b flgs[TR_FLAG_SIZE]) {
    if (p_ptr->lev < 40)
        add_flag(flgs, TR_RES_COLD);
}
static void _steel_birth(void) { 
    p_ptr->current_r_idx = MON_STONE_DRAGON; 
    _dragon_birth();
}
static void _steel_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_STONE_DRAGON && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_STEEL_DRAGON;
        msg_print("You have evolved into a Steel Dragon.");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_steel_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[2] =  {"Stone Dragon", "Steel Dragon"};    
    int           rank = 0;

    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  18,  40,   0,  10,   7,  75,  30};
    skills_t xs = {  8,   7,  15,   0,   0,   0,  30,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 250;

        me.birth = _steel_birth;
        me.calc_bonuses = _steel_calc_bonuses;
        me.get_flags = _steel_get_flags;
        me.get_vulnerabilities = _steel_get_vulnerabilities;
        me.gain_level = _steel_gain_level;
        init = TRUE;
    }

    me.subname = titles[rank];
    me.stats[A_STR] =  5 + (p_ptr->lev / 10);
    me.stats[A_INT] = -6;
    me.stats[A_WIS] = -6;
    me.stats[A_DEX] =  0 - (p_ptr->lev / 10);
    me.stats[A_CON] =  4 + (p_ptr->lev / 10);
    me.stats[A_CHR] =  0 + (p_ptr->lev / 10);
    me.life = 125 + 5*(p_ptr->lev / 10);

    return &me;
}

/**********************************************************************
 * Public
 **********************************************************************/
race_t *mon_dragon_get_race_t(int psubrace)
{
    race_t *result = NULL;

    switch (psubrace)
    {
    case DRAGON_RED:
    case DRAGON_WHITE:
    case DRAGON_BLUE:
    case DRAGON_BLACK:
    case DRAGON_GREEN:
        result = _elemental_get_race_t(psubrace);
        break;
    case DRAGON_NETHER:
        result = _nether_get_race_t();
        break;
    case DRAGON_LAW:
        result = _law_get_race_t();
        break;
    case DRAGON_CHAOS:
        result = _chaos_get_race_t();
        break;
    case DRAGON_BALANCE:
        result = _balance_get_race_t();
        break;
    case DRAGON_ETHEREAL:
        result = _ethereal_get_race_t();
        break;
    case DRAGON_CRYSTAL:
        result = _crystal_get_race_t();
        break;
    case DRAGON_BRONZE:
        result = _bronze_get_race_t();
        break;
    case DRAGON_GOLD:
        result = _gold_get_race_t();
        break;
    case DRAGON_STEEL:
        result = _steel_get_race_t();
        break;
    default: /* Birth Menus */
        result = _nether_get_race_t();
    }

    if (p_ptr->dragon_realm)
    {
        dragon_realm_ptr realm = _get_realm();
        int              i;

        for (i = 0; i < MAX_STATS; i++)
            result->stats[i] += realm->stats[i];

        result->caster_info = _caster_info;
        result->get_spells = _realm_get_spells;
    }
    else
    {
        result->caster_info = NULL;
        result->get_spells = NULL;
    }

    result->name = "Dragon";
    result->desc = _desc;
    result->flags = RACE_IS_MONSTER;
    result->calc_innate_attacks = _calc_innate_attacks;
    result->equip_template = mon_get_equip_template();
    result->base_hp = 40;
    result->pseudo_class_idx = CLASS_ROGUE;

    result->boss_r_idx = MON_GLAURUNG;
    return result;
}


