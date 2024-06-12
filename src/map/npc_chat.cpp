// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifdef PCRE_SUPPORT

#include "npc.hpp"

#include <pcre.h>

#include <common/malloc.hpp>
#include <common/showmsg.hpp>
#include <common/strlib.hpp>
#include <common/timer.hpp>

#include "mob.hpp" // struct mob_data
#include "pc.hpp" // map_session_data

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
 *  set ($@p1$ through $@p9$  with $@p0$ being the entire string)
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

/* Structure containing all info associated with a single pattern block */
struct pcrematch_entry {
	struct pcrematch_entry* next;
	char* pattern;
	pcre* pcre_;
	pcre_extra* pcre_extra_;
	char* label;
};

/* A set of patterns that can be activated and deactived with a single command */
struct pcrematch_set {
	struct pcrematch_set* prev;
	struct pcrematch_set* next;
	struct pcrematch_entry* head;
	int64 setid;
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
	struct pcrematch_set* active;
	struct pcrematch_set* inactive;
};


/**
 * delete everythign associated with a entry
 *
 * This does NOT do the list management
 */
void finalize_pcrematch_entry(struct pcrematch_entry* e)
{
	pcre_free(e->pcre_);
	pcre_free(e->pcre_extra_);
	aFree(e->pattern);
	aFree(e->label);
}

/**
 * Lookup (and possibly create) a new set of patterns by the set id
 */
static struct pcrematch_set* lookup_pcreset(struct npc_data* nd, int64 setid) 
{
	struct pcrematch_set *pcreset;
	struct npc_parse *npcParse = (struct npc_parse *) nd->chatdb;
	if (npcParse == nullptr) 
		nd->chatdb = npcParse = (struct npc_parse *) aCalloc(sizeof(struct npc_parse), 1);
	
	pcreset = npcParse->active;
	
	while (pcreset != nullptr) {
		if (pcreset->setid == setid)
		break;
		pcreset = pcreset->next;
	}
	if (pcreset == nullptr) 
		pcreset = npcParse->inactive;
	
	while (pcreset != nullptr) {
		if (pcreset->setid == setid)
		break;
		pcreset = pcreset->next;
	}
	
	if (pcreset == nullptr) {
		pcreset = (struct pcrematch_set *) aCalloc(sizeof(struct pcrematch_set), 1);
		pcreset->next = npcParse->inactive;
		if (pcreset->next != nullptr)
			pcreset->next->prev = pcreset;
		pcreset->prev = 0;
		npcParse->inactive = pcreset;
		pcreset->setid = setid;
	}
	
	return pcreset;
}

/**
 * activate a set of patterns.
 *
 * if the setid does not exist, this will silently return
 */
static void activate_pcreset(struct npc_data* nd, int64 setid)
{
	struct pcrematch_set *pcreset;
	struct npc_parse *npcParse = (struct npc_parse *) nd->chatdb;
	if (npcParse == nullptr) 
		return; // Nothing to activate...
	pcreset = npcParse->inactive;
	while (pcreset != nullptr) {
		if (pcreset->setid == setid)
			break;
		pcreset = pcreset->next;
	}
	if (pcreset == nullptr)
		return; // not in inactive list
	if (pcreset->next != nullptr)
		pcreset->next->prev = pcreset->prev;
	if (pcreset->prev != nullptr)
		pcreset->prev->next = pcreset->next;
	else 
		npcParse->inactive = pcreset->next;
	
	pcreset->prev = nullptr;
	pcreset->next = npcParse->active;
	if (pcreset->next != nullptr)
		pcreset->next->prev = pcreset;
	npcParse->active = pcreset;
}

/**
 * deactivate a set of patterns.
 *
 * if the setid does not exist, this will silently return
 */
static void deactivate_pcreset(struct npc_data* nd, int64 setid)
{
	struct pcrematch_set *pcreset;
	struct npc_parse *npcParse = (struct npc_parse *) nd->chatdb;
	if (npcParse == nullptr) 
		return; // Nothing to deactivate...
	if (setid == -1) {
		while(npcParse->active != nullptr)
			deactivate_pcreset(nd, npcParse->active->setid);
		return;
	}
	pcreset = npcParse->active;
	while (pcreset != nullptr) {
		if (pcreset->setid == setid)
			break;
		pcreset = pcreset->next;
	}
	if (pcreset == nullptr)
		return; // not in active list
	if (pcreset->next != nullptr)
		pcreset->next->prev = pcreset->prev;
	if (pcreset->prev != nullptr)
		pcreset->prev->next = pcreset->next;
	else 
		npcParse->active = pcreset->next;
	
	pcreset->prev = nullptr;
	pcreset->next = npcParse->inactive;
	if (pcreset->next != nullptr)
		pcreset->next->prev = pcreset;
	npcParse->inactive = pcreset;
}

/**
 * delete a set of patterns.
 */
static void delete_pcreset(struct npc_data* nd, int64 setid)
{
	int active = 1;
	struct pcrematch_set *pcreset;
	struct npc_parse *npcParse = (struct npc_parse *) nd->chatdb;
	if (npcParse == nullptr) 
		return; // Nothing to deactivate...
	pcreset = npcParse->active;
	while (pcreset != nullptr) {
		if (pcreset->setid == setid)
			break;
		pcreset = pcreset->next;
	}
	if (pcreset == nullptr) {
		active = 0;
		pcreset = npcParse->inactive;
		while (pcreset != nullptr) {
			if (pcreset->setid == setid)
				break;
			pcreset = pcreset->next;
		}
	}
	if (pcreset == nullptr) 
		return;
	
	if (pcreset->next != nullptr)
		pcreset->next->prev = pcreset->prev;
	if (pcreset->prev != nullptr)
		pcreset->prev->next = pcreset->next;
	
	if(active)
		npcParse->active = pcreset->next;
	else
		npcParse->inactive = pcreset->next;
	
	pcreset->prev = nullptr;
	pcreset->next = nullptr;
	
	while (pcreset->head) {
		struct pcrematch_entry* n = pcreset->head->next;
		finalize_pcrematch_entry(pcreset->head);
		aFree(pcreset->head); // Cleanin' the last ones.. [Lance]
		pcreset->head = n;
	}
	
	aFree(pcreset);
}

/**
 * create a new pattern entry 
 */
static struct pcrematch_entry* create_pcrematch_entry(struct pcrematch_set* set)
{
	struct pcrematch_entry * e =  (struct pcrematch_entry *) aCalloc(sizeof(struct pcrematch_entry), 1);
	struct pcrematch_entry * last = set->head;
	
	// Normally we would have just stuck it at the end of the list but
	// this doesn't sink up with peoples usage pattern.  They wanted
	// the items defined first to have a higher priority then the
	// items defined later. as a result, we have to do some work up front.
	
	/*  if we are the first pattern, stick us at the end */
	if (last == nullptr) {
		set->head = e;
		return e;
	}
	
	/* Look for the last entry */
	while (last->next != nullptr)
		last = last->next;
	
	last->next = e;
	e->next = nullptr;
	
	return e;
}

/**
 * define/compile a new pattern
 */
void npc_chat_def_pattern(struct npc_data* nd, int64 setid, const char* pattern, const char* label)
{
	const char *err;
	int erroff;
	
	struct pcrematch_set * s = lookup_pcreset(nd, setid);
	struct pcrematch_entry *e = create_pcrematch_entry(s);
	e->pattern = aStrdup(pattern);
	e->label = aStrdup(label);
	e->pcre_ = pcre_compile(pattern, PCRE_CASELESS, &err, &erroff, nullptr);
	e->pcre_extra_ = pcre_study(e->pcre_, 0, &err);
}

/**
 * Delete everything associated with a NPC concerning the pattern
 * matching code 
 *
 * this could be more efficent but.. how often do you do this?
 */
void npc_chat_finalize(struct npc_data* nd)
{
	struct npc_parse *npcParse = (struct npc_parse *) nd->chatdb;
	if (npcParse == nullptr)
		return;
	
	while(npcParse->active)
		delete_pcreset(nd, npcParse->active->setid);
	
	while(npcParse->inactive)
		delete_pcreset(nd, npcParse->inactive->setid);
	
	// Additional cleaning up [Lance]
	aFree(npcParse);
}

/**
 * Handler called whenever a global message is spoken in a NPC's area
 */
int npc_chat_sub(struct block_list* bl, va_list ap)
{
	struct npc_data* nd = (struct npc_data *) bl;
	struct npc_parse* npcParse = (struct npc_parse *) nd->chatdb;
	char* msg;
	int len, i;
	map_session_data* sd;
	struct npc_label_list* lst;
	struct pcrematch_set* pcreset;
	struct pcrematch_entry* e;
	
	// Not interested in anything you might have to say...
	if (npcParse == nullptr || npcParse->active == nullptr)
		return 0;
	
	msg = va_arg(ap,char*);
	len = va_arg(ap,int);
	sd = va_arg(ap,map_session_data *);
	
	// iterate across all active sets
	for (pcreset = npcParse->active; pcreset != nullptr; pcreset = pcreset->next)
	{
		// interate across all patterns in that set
		for (e = pcreset->head; e != nullptr; e = e->next)
		{
			int offsets[2*10 + 10]; // 1/3 reserved for temp space requred by pcre_exec
			
			// perform pattern match
			int r = pcre_exec(e->pcre_, e->pcre_extra_, msg, len, 0, 0, offsets, ARRAYLENGTH(offsets));
			if (r > 0)
			{
				// save out the matched strings
				for (i = 0; i < r; i++)
				{
					char var[255], val[255];
					snprintf(var, sizeof(var), "$@p%i$", i);
					pcre_copy_substring(msg, offsets, r, i, val, sizeof(val));
					set_var_str( sd, var, val );
				}
				
				// find the target label.. this sucks..
				lst = nd->u.scr.label_list;
				ARR_FIND(0, nd->u.scr.label_list_num, i, strncmp(lst[i].name, e->label, sizeof(lst[i].name)) == 0);
				if (i == nd->u.scr.label_list_num) {
					ShowWarning("Unable to find label: %s\n", e->label);
					return 0;
				}
				
				// run the npc script
				run_script(nd->u.scr.script,lst[i].pos,sd->bl.id,nd->bl.id);
				return 0;
			}
		}
	}
	
	return 0;
}

// Various script builtins used to support these functions

int buildin_defpattern(struct script_state* st)
{
	int64 setid = conv_num64(st,& (st->stack->stack_data[st->start+2]));
	const char* pattern = conv_str(st,& (st->stack->stack_data[st->start+3]));
	const char* label = conv_str(st,& (st->stack->stack_data[st->start+4]));
	struct npc_data* nd = (struct npc_data *)map_id2bl(st->oid);
	
	npc_chat_def_pattern(nd, setid, pattern, label);
	
	return 0;
}

int buildin_activatepset(struct script_state* st)
{
	int64 setid = conv_num64(st,& (st->stack->stack_data[st->start+2]));
	struct npc_data* nd = (struct npc_data *)map_id2bl(st->oid);
	
	activate_pcreset(nd, setid);
	
	return 0;
}

int buildin_deactivatepset(struct script_state* st)
{
	int64 setid = conv_num64(st,& (st->stack->stack_data[st->start+2]));
	struct npc_data* nd = (struct npc_data *)map_id2bl(st->oid);
	
	deactivate_pcreset(nd, setid);
	
	return 0;
}

int buildin_deletepset(struct script_state* st)
{
	int64 setid = conv_num64(st,& (st->stack->stack_data[st->start+2]));
	struct npc_data* nd = (struct npc_data *)map_id2bl(st->oid);
	
	delete_pcreset(nd, setid);
	
	return 0;
}

#endif //PCRE_SUPPORT
