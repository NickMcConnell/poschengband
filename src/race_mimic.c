#include "angband.h"

static int _calc_level(int l)
{
    return l + l*l*l/2500;
}

static void _set_current_r_idx(int r_idx)
{
    possessor_set_current_r_idx(r_idx);
    /* Mimics shift forms often enough to be annoying if shapes
       have dramatically different body types (e.g. dragons vs humanoids).
       Inscribe gear with @mimic to autoequip on shifing. */
    equip_shuffle("@mimic");
}

static void _birth(void) 
{ 
    object_type forge;

    p_ptr->current_r_idx = MON_MIMIC;
    equip_on_change_race();
    
    object_prep(&forge, lookup_kind(TV_SWORD, SV_LONG_SWORD));
    add_outfit(&forge);

    object_prep(&forge, lookup_kind(TV_SOFT_ARMOR, SV_LEATHER_SCALE_MAIL));
    add_outfit(&forge);

    object_prep(&forge, lookup_kind(TV_RING, 0));
    forge.name2 = EGO_RING_COMBAT;
    forge.to_d = 3;
    add_outfit(&forge);
}

static bool _is_memorized(int r_idx)
{
    return FALSE;
}

static bool _is_visible(int r_idx)
{
    int i;
    for (i = 1; i < m_max; i++)
    {
        monster_type *m_ptr = &m_list[i];

        if (m_ptr->r_idx != r_idx) continue;
        if (!projectable(py, px, m_ptr->fy, m_ptr->fx)) continue;
        return TRUE;
    }
    return FALSE;
}

static void _player_action(int energy_use)
{
    if (possessor_get_toggle() == LEPRECHAUN_TOGGLE_BLINK)
        teleport_player(10, TELEPORT_LINE_OF_SIGHT);

    /* Maintain current form. 
       Rules: If the source is visible, then we can always maintain the form. 
       Otherwise, memorized forms get a saving throw to maintain, but non-memorized 
       forms are simply lost. */
    if ( p_ptr->current_r_idx != MON_MIMIC 
      && one_in_(100) 
      && !_is_visible(p_ptr->current_r_idx) )
    {
        bool lose_form = FALSE;
        if (_is_memorized(p_ptr->current_r_idx))
        {
            int r_lvl = r_info[p_ptr->current_r_idx].level;
            int p_lvl = _calc_level(p_ptr->max_plv);
            p_lvl += 3 + p_ptr->stat_ind[A_DEX];

            if (randint1(p_lvl) < r_lvl)
            {
                msg_print("You lose control over your current form.");
                lose_form = TRUE;
            }
        }
        else
        {
            msg_print("Your knowledge of this form fades from memory.");
            lose_form = TRUE;
        }

        _set_current_r_idx(MON_MIMIC);
    }
}

/**********************************************************************
 * Powers
 **********************************************************************/
static void _mimic_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        if (p_ptr->current_r_idx == MON_MIMIC)
            var_set_string(res, "Mimic");
        else
            var_set_string(res, "Stop Mimicry");
        break;
    case SPELL_DESC:
        if (p_ptr->current_r_idx == MON_MIMIC)
            var_set_string(res, "Mimic a nearby visible monster, gaining the powers and abilities of that form.");
        else
            var_set_string(res, "Return to your native form.");
        break;
    case SPELL_INFO:
        var_set_string(res, format("Lvl %d", _calc_level(p_ptr->max_plv) + 5));
        break;
    case SPELL_CAST:
    {
        var_set_bool(res, FALSE);

        if (p_ptr->current_r_idx == MON_MIMIC)
        {
            int m_idx = 0;
            int r_idx = 0;

            if (target_set(TARGET_MARK))
            {
                msg_flag = FALSE; /* Bug ... we get an extra -more- prompt after target_set() ... */
                if (target_who > 0)
                    m_idx = target_who;
                else
                    m_idx = cave[target_row][target_col].m_idx;
            }
            if (!m_idx) return;

            r_idx = m_list[m_idx].r_idx;
            if (!r_idx) return;

            if (r_info[r_idx].level > _calc_level(p_ptr->max_plv) + 5)
            {
                char m_name[MAX_NLEN];
                monster_desc(m_name, &m_list[m_idx], 0);
                msg_format("You are not powerful enough to mimic %s (Lvl %d).", m_name, r_info[r_idx].level);
            }
            else
                _set_current_r_idx(r_idx);
        }
        else
            _set_current_r_idx(MON_MIMIC);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _add_power(spell_info* spell, int lvl, int cost, int fail, ang_spell fn, int stat_idx)
{
    spell->level = lvl;
    spell->cost = cost;
    spell->fail = calculate_fail_rate(lvl, fail, stat_idx); 
    spell->fn = fn;
}

static int _get_powers(spell_info* spells, int max)
{
    int ct = 0;

    if (ct < max)
        _add_power(&spells[ct++], 1, 0, 0, _mimic_spell, p_ptr->stat_ind[A_DEX]);

    ct += possessor_get_powers(spells + ct, max - ct);
    return ct;
}

/**********************************************************************
 * Public
 **********************************************************************/
race_t *mon_mimic_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;

    if (!init)
    {
        me.name = "Mimic";
        me.desc = "Mimics are similar to possessors but instead of controlling the corpses of the "
                    "vanquished, the mimic imitates those about them. This allows the mimic to assume "
                    "the forms of foes they have yet to conquer and is quite useful. However, there is "
                    "a small catch: The mimic can only copy what they see! This limitation forces the "
                    "mimic to change forms much more often than the possessor would as knowledge of their "
                    "current body rapidly fades when the original is no longer about. Occasionally, the "
                    "mimic is able to memorize a particular form well enough to use it again without the "
                    "original body to imitate, though this does not happen very often and the mimic can "
                    "only memorize a small number of forms. To have a chance of this, the mimic must be "
                    "in the desired form when slaying the original.\n \n"
                    "Mimics are monsters and do not choose a normal class. Their stats, skills, resistances "
                    "and spells are determined by the form they assume. Their current body also "
                    "determines their spell stat (e.g. a novice priest uses wisdom, a novice mage uses intelligence). "
                    "Their current body may offer innate powers (e.g. breath weapons or rockets) in addition to or in lieu "
                    "of magical powers (e.g. mana storms and frost bolts). Be sure to check both the racial power "
                    "command ('U') and the magic command ('m') after assuming a new body.";

        me.exp = 250;

        me.birth = _birth;

        me.get_powers = _get_powers;

        me.calc_bonuses = possessor_calc_bonuses;
        me.get_flags = possessor_get_flags;
        me.get_immunities = possessor_get_immunities;
        me.get_vulnerabilities = possessor_get_vulnerabilities;
        me.player_action = _player_action;
        
        me.calc_innate_attacks = possessor_calc_innate_attacks;

        me.flags = RACE_IS_MONSTER;
        me.boss_r_idx = MON_CHAMELEON_K;

        init = TRUE;
    }
    possessor_init_race_t(&me, MON_MIMIC);
    return &me;
}

void mimic_dispel_player(void)
{
    if (p_ptr->prace != RACE_MON_MIMIC) return;
    _set_current_r_idx(MON_MIMIC);
}
