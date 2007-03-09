// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifdef PCRE_SUPPORT

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "../common/timer.h"
#include "../common/malloc.h"
#include "../common/version.h"
#include "../common/nullpo.h"
#include "../common/showmsg.h"

#include "map.h"
#include "status.h"
#include "npc.h"
#include "chat.h"
#include "script.h"
#include "battle.h"

#include "pcre.h"

/**
 *  Written by MouseJstr in a vision... (2/21/2005)
 *
 *  This allows you to make npc listen for spoken text (global
 *  messages) and pattern match against that spoken text using perl
 *  regular expressions.
 *
 *  Please feel free to copy this code into your own personal ragnarok
 *  servers or distributions but please leave my name.  Also, please
 *  wait until I've put it into the main eA branch which means I
 *  believe it is ready for distribution.
 *
 *  So, how do people use this?
 *
 *  The first and most important function is defpattern
 *
 *    defpattern 1, "[^:]+: (.*) loves (.*)", "label";
 *
 *  this defines a new pattern in set 1 using perl syntax 
 *    (http://www.troubleshooters.com/codecorn/littperl/perlreg.htm)
 *  and tells it to jump to the supplied label when the pattern
 *  is matched.
 *
 *  each of the matched Groups will result in a variable being
 *  set ($p1$ through $p9$  with $p0$ being the entire string)
 *  before the script gets executed.
 *
 *    activatepset 1;
 * 
 *  This activates a set of patterns.. You can have many pattern
 *  sets defined and many active all at once.  This feature allows
 *  you to set up "conversations" and ever changing expectations of
 *  the pattern matcher
 *
 *    deactivatepset 1;
 *
 *  turns off a pattern set;
 *
 *    deactivatepset -1;
 *
 *  turns off ALL pattern sets;
 *
 *    deletepset 1;
 *
 *  deletes a pset
 */

/* Structure containing all info associated with a single pattern
   block */

struct pcrematch_entry {
    struct pcrematch_entry *next_;
    char *pattern_;
    pcre *pcre_;
    pcre_extra *pcre_extra_;
    char *label_;
};

/* A set of patterns that can be activated and deactived with a single
   command */

struct pcrematch_set {
    struct pcrematch_set *next_, *prev_;
    struct pcrematch_entry *head_;
    int setid_;
};

/* 
 * Entire data structure hung off a NPC
 *
 * The reason I have done it this way (a void * in npc_data and then
 * this) was to reduce the number of patches that needed to be applied
 * to a ragnarok distribution to bring this code online.  I
 * also wanted people to be able to grab this one file to get updates
 * without having to do a large number of changes.
 */

struct npc_parse {
    struct pcrematch_set *active_;
    struct pcrematch_set *inactive_;
};


/**
 * delete everythign associated with a entry
 *
 * This does NOT do the list management
 */

void finalize_pcrematch_entry(struct pcrematch_entry *e) {
//TODO: For some odd reason this causes a already-free'd error under Windows, but not *nix! [Skotlex]
#ifndef _WIN32
	if (e->pcre_) {
		free(e->pcre_);
		e->pcre_ = NULL;
	}
#endif
	if (e->pcre_extra_) {
		free(e->pcre_extra_);
		e->pcre_ = NULL;
	}
	aFree(e->pattern_);
	aFree(e->label_);
}

/**
 * Lookup (and possibly create) a new set of patterns by the set id
 */
static struct pcrematch_set * lookup_pcreset(struct npc_data *nd,int setid) 
{
    struct pcrematch_set *pcreset;
    struct npc_parse *npcParse = (struct npc_parse *) nd->chatdb;
    if (npcParse == NULL) 
        nd->chatdb = npcParse = (struct npc_parse *)
            aCalloc(sizeof(struct npc_parse), 1);

    pcreset = npcParse->active_;

    while (pcreset != NULL) {
        if (pcreset->setid_ == setid)
            break;
        pcreset = pcreset->next_;
    }
    if (pcreset == NULL) 
        pcreset = npcParse->inactive_;

    while (pcreset != NULL) {
        if (pcreset->setid_ == setid)
            break;
        pcreset = pcreset->next_;
    }

    if (pcreset == NULL) {
        pcreset = (struct pcrematch_set *) 
            aCalloc(sizeof(struct pcrematch_set), 1);
        pcreset->next_ = npcParse->inactive_;
        if (pcreset->next_ != NULL)
            pcreset->next_->prev_ = pcreset;
        pcreset->prev_ = 0;
        npcParse->inactive_ = pcreset;
        pcreset->setid_ = setid;
    }

    return pcreset;
}

/**
 * activate a set of patterns.
 *
 * if the setid does not exist, this will silently return
 */

static void activate_pcreset(struct npc_data *nd,int setid) {
    struct pcrematch_set *pcreset;
    struct npc_parse *npcParse = (struct npc_parse *) nd->chatdb;
    if (npcParse == NULL) 
        return; // Nothing to activate...
    pcreset = npcParse->inactive_;
    while (pcreset != NULL) {
        if (pcreset->setid_ == setid)
            break;
        pcreset = pcreset->next_;
    }
    if (pcreset == NULL)
        return; // not in inactive list
    if (pcreset->next_ != NULL)
        pcreset->next_->prev_ = pcreset->prev_;
    if (pcreset->prev_ != NULL)
        pcreset->prev_->next_ = pcreset->next_;
    else 
        npcParse->inactive_ = pcreset->next_;

    pcreset->prev_ = NULL;
    pcreset->next_ = npcParse->active_;
    if (pcreset->next_ != NULL)
        pcreset->next_->prev_ = pcreset;
    npcParse->active_ = pcreset;
}

/**
 * deactivate a set of patterns.
 *
 * if the setid does not exist, this will silently return
 */

static void deactivate_pcreset(struct npc_data *nd,int setid) {
    struct pcrematch_set *pcreset;
    struct npc_parse *npcParse = (struct npc_parse *) nd->chatdb;
    if (npcParse == NULL) 
        return; // Nothing to deactivate...
    if (setid == -1) {
      while(npcParse->active_ != NULL)
        deactivate_pcreset(nd, npcParse->active_->setid_);
      return;
    }
    pcreset = npcParse->active_;
    while (pcreset != NULL) {
        if (pcreset->setid_ == setid)
            break;
        pcreset = pcreset->next_;
    }
    if (pcreset == NULL)
        return; // not in active list
    if (pcreset->next_ != NULL)
        pcreset->next_->prev_ = pcreset->prev_;
    if (pcreset->prev_ != NULL)
        pcreset->prev_->next_ = pcreset->next_;
    else 
        npcParse->active_ = pcreset->next_;

    pcreset->prev_ = NULL;
    pcreset->next_ = npcParse->inactive_;
    if (pcreset->next_ != NULL)
        pcreset->next_->prev_ = pcreset;
    npcParse->inactive_ = pcreset;
}

/**
 * delete a set of patterns.
 */
static void delete_pcreset(struct npc_data *nd,int setid) {
    int active = 1;
    struct pcrematch_set *pcreset;
    struct npc_parse *npcParse = (struct npc_parse *) nd->chatdb;
    if (npcParse == NULL) 
        return; // Nothing to deactivate...
    pcreset = npcParse->active_;
    while (pcreset != NULL) {
        if (pcreset->setid_ == setid)
            break;
        pcreset = pcreset->next_;
    }
    if (pcreset == NULL) {
        active = 0;
    	pcreset = npcParse->inactive_;
    	while (pcreset != NULL) {
        	if (pcreset->setid_ == setid)
            	break;
        	pcreset = pcreset->next_;
    	}
    }
    if (pcreset == NULL) 
	return;
        
    if (pcreset->next_ != NULL)
        pcreset->next_->prev_ = pcreset->prev_;
    if (pcreset->prev_ != NULL)
        pcreset->prev_->next_ = pcreset->next_;

	if(active)
		npcParse->active_ = pcreset->next_;
	else
		npcParse->inactive_ = pcreset->next_;

    pcreset->prev_ = NULL;
    pcreset->next_ = NULL;

    while (pcreset->head_) {
		struct pcrematch_entry *n = pcreset->head_->next_;
		finalize_pcrematch_entry(pcreset->head_);
		aFree(pcreset->head_); // Cleanin' the last ones.. [Lance]
		pcreset->head_ = n;
    }

	aFree(pcreset);
}

/**
 * create a new pattern entry 
 */
static struct pcrematch_entry *create_pcrematch_entry(struct pcrematch_set * set) {
    struct pcrematch_entry * e =  (struct pcrematch_entry *)
        aCalloc(sizeof(struct pcrematch_entry), 1);
    struct pcrematch_entry * last = set->head_;

    // Normally we would have just stuck it at the end of the list but
    // this doesn't sink up with peoples usage pattern.  They wanted
    // the items defined first to have a higher priority then the
    // items defined later.. as a result, we have to do some work up
    // front..

    /*  if we are the first pattern, stick us at the end */
    if (last == NULL) {
        set->head_ = e;
        return e;
    }

    /* Look for the last entry */
    while (last->next_ != NULL)
        last = last->next_;

    last->next_ = e;
    e->next_ = NULL;

    return e;
}

/**
 * define/compile a new pattern
 */

void npc_chat_def_pattern(struct npc_data *nd, int setid, 
    const char *pattern, const char *label)
{
    const char *err;
    int erroff;

    struct pcrematch_set * s = lookup_pcreset(nd, setid);
    struct pcrematch_entry *e = create_pcrematch_entry(s);
    e->pattern_ = aStrdup(pattern);
    e->label_ = aStrdup(label);
    e->pcre_ = pcre_compile(pattern, PCRE_CASELESS, &err, &erroff, NULL);
    e->pcre_extra_ = pcre_study(e->pcre_, 0, &err);
}

/**
 * Delete everything associated with a NPC concerning the pattern
 * matching code 
 *
 * this could be more efficent but.. how often do you do this?
 */
void npc_chat_finalize(struct npc_data *nd)
{
    struct npc_parse *npcParse = (struct npc_parse *) nd->chatdb;
    if (npcParse == NULL)
        return;

    while(npcParse->active_)
      delete_pcreset(nd, npcParse->active_->setid_);

    while(npcParse->inactive_)
      delete_pcreset(nd, npcParse->inactive_->setid_);

	// Additional cleaning up [Lance]
	aFree(npcParse);
}

/**
 * Handler called whenever a global message is spoken in a NPC's area
 */
int npc_chat_sub(struct block_list *bl, va_list ap)
{
    struct npc_data *nd = (struct npc_data *)bl;
    struct npc_parse *npcParse = (struct npc_parse *) nd->chatdb;
    unsigned char *msg;
    int len, pos, i;
    struct map_session_data *sd;
    struct npc_label_list *lst;
    struct pcrematch_set *pcreset;

    // Not interested in anything you might have to say...
    if (npcParse == NULL || npcParse->active_ == NULL)
        return 0;

    msg = va_arg(ap,unsigned char*);
    len = va_arg(ap,int);
    sd = va_arg(ap,struct map_session_data *);

    // grab the active list
    pcreset = npcParse->active_;

    // interate across all active sets
    while (pcreset != NULL) {
        struct pcrematch_entry *e = pcreset->head_;
        // interate across all patterns in that set
        while (e != NULL) {
            int offsets[20];
            char buf[255];
            // perform pattern match
            int r = pcre_exec(e->pcre_, e->pcre_extra_, msg, len, 0, 
                0, offsets, sizeof(offsets) / sizeof(offsets[0]));
            if (r >= 0) {
                // save out the matched strings
                switch (r) {
                case 10:
                    memcpy(buf, &msg[offsets[18]], offsets[19]);
                    buf[offsets[19]] = '\0';
                    set_var(sd, "$p9$", buf);
                case 9:
                    memcpy(buf, &msg[offsets[16]], offsets[17]);
                    buf[offsets[17]] = '\0';
                    set_var(sd, "$p8$", buf);
                case 8:
                    memcpy(buf, &msg[offsets[14]], offsets[15]);
                    buf[offsets[15]] = '\0';
                    set_var(sd, "$p7$", buf);
                case 7:
                    memcpy(buf, &msg[offsets[12]], offsets[13]);
                    buf[offsets[13]] = '\0';
                    set_var(sd, "$p6$", buf);
                case 6:
                    memcpy(buf, &msg[offsets[10]], offsets[11]);
                    buf[offsets[11]] = '\0';
                    set_var(sd, "$p5$", buf);
                case 5:
                    memcpy(buf, &msg[offsets[8]], offsets[9]);
                    buf[offsets[9]] = '\0';
                    set_var(sd, "$p4$", buf);
                case 4:
                    memcpy(buf, &msg[offsets[6]], offsets[7]);
                    buf[offsets[7]] = '\0';
                    set_var(sd, "$p3$", buf);
                case 3:
                    memcpy(buf, &msg[offsets[4]], offsets[5]);
                    buf[offsets[5]] = '\0';
                    set_var(sd, "$p2$", buf);
                case 2:
                    memcpy(buf, &msg[offsets[2]], offsets[3]);
                    buf[offsets[3]] = '\0';
                    set_var(sd, "$p1$", buf);
                case 1:
                    memcpy(buf, &msg[offsets[0]], offsets[1]);
                    buf[offsets[1]] = '\0';
                    set_var(sd, "$p0$", buf);
                }

                // find the target label.. this sucks..
                lst=nd->u.scr.label_list;
                pos = -1;
                for (i = 0; i < nd->u.scr.label_list_num; i++) {
                    if (strncmp(lst[i].name, e->label_, sizeof(lst[i].name)) == 0) {
                        pos = lst[i].pos;
                        break;
                    }
                }
                if (pos == -1) {
                    ShowWarning("Unable to find label: %s", e->label_);
                    // unable to find label... do something..
                    return 0;
                }
                // run the npc script
                run_script(nd->u.scr.script,pos,sd->bl.id,nd->bl.id);
                return 0;
            }
            e = e->next_;
        }
        pcreset = pcreset->next_;
    }

    return 0;
}

int mob_chat_sub(struct block_list *bl, va_list ap){
	struct mob_data *md = (struct mob_data *)bl;
	if(md->nd){
		npc_chat_sub(&md->nd->bl, ap);
	}
	return 0;
}

// Various script builtins used to support these functions

int buildin_defpattern(struct script_state *st) {
    int setid=conv_num(st,& (st->stack->stack_data[st->start+2]));
    const char *pattern=conv_str(st,& (st->stack->stack_data[st->start+3]));
    const char *label=conv_str(st,& (st->stack->stack_data[st->start+4]));
    struct npc_data *nd=(struct npc_data *)map_id2bl(st->oid);
    
    npc_chat_def_pattern(nd, setid, pattern, label);

    return 0;
}

int buildin_activatepset(struct script_state *st) {
    int setid=conv_num(st,& (st->stack->stack_data[st->start+2]));
    struct npc_data *nd=(struct npc_data *)map_id2bl(st->oid);

    activate_pcreset(nd, setid);

    return 0;
}
int buildin_deactivatepset(struct script_state *st) {
    int setid=conv_num(st,& (st->stack->stack_data[st->start+2]));
    struct npc_data *nd=(struct npc_data *)map_id2bl(st->oid);

    deactivate_pcreset(nd, setid);

    return 0;
}
int buildin_deletepset(struct script_state *st) {
    int setid=conv_num(st,& (st->stack->stack_data[st->start+2]));
    struct npc_data *nd=(struct npc_data *)map_id2bl(st->oid);

    delete_pcreset(nd, setid);

    return 0;
}


#endif
