#include "angband.h"

/**********************************************************************
 * Memorized Forms
 **********************************************************************/
#define _MAX_FORMS 5
static int _forms[_MAX_FORMS];

static bool _is_memorized(int r_idx)
{
    int i;
    for (i = 0; i < _MAX_FORMS; i++)
    {
        if (_forms[i] == r_idx)
            return TRUE;
    }
    return FALSE;
}

static int _count_memorized(void)
{
    int ct = 0, i;

    for (i = 0; i < _MAX_FORMS; i++)
    {
        if (_forms[i])
            ct++;
    }
    return ct;
}

/* UI code is yucky ... There are two scenarios, prompting for
   an existing learned form when using the mimicry talent, and 
   prompting for a slot (which may or may not be empty) when
   learning a new form. In the first case, we should skip empty
   slots and the user may opt to target a visible monster instead
   (i.e. use an unlearned form). In the second case, we should
   show empty slots and there is no option to choose a target.
   In both cases, the user may cancel the UI command altogether.
*/
#define _PROMPT_CANCEL  -1  /* Return code indicating cancel */
#define _PROMPT_TARGET  -2  /* Return code indicating desire to target */
#define _PROMPT_EXISTING 1  /* Only show learned forms */
#define _PROMPT_ANY      2  /* Show empty slots as well */


static void _menu_fn(int cmd, int which, vptr cookie, variant *res)
{
    int  slot = ((int*)cookie)[which];
    int  r_idx = 0;

    if (0 <= slot && slot < _MAX_FORMS)
        r_idx = _forms[slot];

    switch (cmd)
    {
    case MENU_TEXT:
        if (slot == _PROMPT_TARGET)
            var_set_string(res, "Choose Target");
        else if (r_idx)
            var_set_string(res, r_name + r_info[r_idx].name);
        else
            var_set_string(res, "Unused");
        break;
    case MENU_COLOR:
        if (slot == _PROMPT_TARGET)
            var_set_int(res, TERM_UMBER);
        else if (r_idx)
            var_set_int(res, TERM_WHITE);
        else
            var_set_int(res, TERM_L_DARK);
        break;
    case MENU_KEY:
        if (slot == _PROMPT_TARGET)
            var_set_int(res, 't');
        break;        
    }
}

static int _prompt_memorized_aux(int mode) /* returns the slot chosen! */
{
    int result = _PROMPT_CANCEL;
    int ct = 0, i;
    int choices[_MAX_FORMS + 1];

    for (i = 0; i < _MAX_FORMS; i++)
    {
        if (mode == _PROMPT_ANY || _forms[i])
            choices[ct++] = i;
    }

    if (ct && mode == _PROMPT_EXISTING)
        choices[ct++] = _PROMPT_TARGET;

    /* TODO: Give a nice custom menu similar to the monster knowledge screen */
    if (ct)
    {
        menu_t menu = { (mode == _PROMPT_EXISTING) ? "Mimic which form?" : "Choose a slot for this form.", 
                            NULL, NULL, _menu_fn,  choices, ct};
        int    idx = menu_choose(&menu);
        
        if (idx >= 0)
            result = choices[idx];
    }

    return result;
}

static int _prompt_memorized(void)
{
    int r_idx = _PROMPT_TARGET;
    if (_count_memorized())
    {
        int i = _prompt_memorized_aux(_PROMPT_EXISTING);
        if (i >= 0 && i < _MAX_FORMS)
            r_idx = _forms[i];
        else
            r_idx = i; /* Special code to choose target or escape the command */
    }
    return r_idx;
}

static bool _memorize_form(int r_idx)
{
    int           i;
    monster_race *r_ptr = &r_info[r_idx];

    if (_is_memorized(r_idx))
    {
        msg_format("You already know this form (%s).", r_name + r_ptr->name);
        return FALSE;
    }

    i = _prompt_memorized_aux(_PROMPT_ANY);
    if (i >= 0 && i < _MAX_FORMS)
    {
        if (_forms[i])
        {
            int           r_idx2 = _forms[i];
            monster_race *r_ptr2 = &r_info[r_idx2];
            char          prompt[512];

            sprintf(prompt, "Really replace %s with %s? ", r_name + r_ptr2->name, r_name + r_ptr->name);
            if (!get_check(prompt))
                return FALSE;
        }

        _forms[i] = r_idx;
        msg_format("You have learned this form (%s).", r_name + r_ptr->name);
        return TRUE;
    }
    return FALSE;
}

static int _prompt(void)
{
    int r_idx = _prompt_memorized();

    if (r_idx == _PROMPT_TARGET)
    {
        int m_idx = 0;
        if (target_set(TARGET_MARK))
        {
            msg_flag = FALSE; /* Bug ... we get an extra -more- prompt after target_set() ... */
            if (target_who > 0)
                m_idx = target_who;
            else
                m_idx = cave[target_row][target_col].m_idx;
        }
        if (m_idx)
        {
            monster_type *m_ptr = &m_list[m_idx];
            r_idx = m_ptr->r_idx;
            if (!projectable(py, px, m_ptr->fy, m_ptr->fx))
            {
                msg_print("You must be able to see your target to mimic.");
                r_idx = _PROMPT_CANCEL;
            }
        }
        else
            r_idx = _PROMPT_CANCEL;
    }
    return r_idx;
}

static void _load(savefile_ptr file)
{
    int ct, i;

    for (i = 0; i < _MAX_FORMS; i++)
        _forms[i] = 0;

    ct = savefile_read_s16b(file);
    for (i = 0; i < ct; i++)
    {
        int r_idx = savefile_read_s16b(file);
        if (i < _MAX_FORMS)
            _forms[i] = r_idx;
    }
}

static void _save(savefile_ptr file)
{
    int i;

    savefile_write_s16b(file, _count_memorized());

    for (i = 0; i < _MAX_FORMS; i++)
    {
        if (_forms[i])
            savefile_write_s16b(file, _forms[i]);
    }
}

/**********************************************************************
 * Utilities
 **********************************************************************/
static int _calc_level(int l)
{
    return l + l*l*l/2500;
}

static void _set_current_r_idx(int r_idx)
{
    disturb(1, 0);
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
            int p_lvl = _calc_level(p_ptr->max_plv); /* Use max level in case player is assuming a weak form that decreases player level. */
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

        if (lose_form)
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
        if (p_ptr->current_r_idx == MON_MIMIC)
            var_set_string(res, format("Lvl %d", _calc_level(p_ptr->lev) + 5));
        break;
    case SPELL_CAST:
    {
        var_set_bool(res, FALSE);

        if (p_ptr->current_r_idx == MON_MIMIC)
        {
            int           r_idx = _prompt();
            monster_race *r_ptr = 0;

            if (r_idx <= 0 || r_idx > max_r_idx) return;

            r_ptr = &r_info[r_idx];
            if (r_ptr->level > _calc_level(p_ptr->lev) + 5)
                msg_format("You are not powerful enough to mimic this form (%s: Lvl %d).", r_name + r_ptr->name, r_ptr->level);
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

        me.load_player = _load;
        me.save_player = _save;
        
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

void mimic_on_kill_monster(int r_idx)
{
    if (p_ptr->prace != RACE_MON_MIMIC) return;

    /* To learn a form, you must be mimicking it when you land the killing blow. */
    if (r_idx != p_ptr->current_r_idx) return;

    /*          v---- Tweak odds, but this should work for now */
    if (one_in_(20) && _memorize_form(r_idx))
    {
    }
}