//===== rAthena Script ===========================================|
//= Episode 19 - The Land of Snow Flower, Isgard                 =|
//===== By: ======================================================|
//= crazyarashi                                                  =|
//===== Current Version: =========================================|
//= 1.0 Initial Version                                          =|
//= 1.1 Bug Fixes Cleanup                                        =|
//================================================================|

wolfvill,223,83,3	script	Grey Wolf Villager#ep19	4_EP18_GW_MAN02,{
	if(isbegin_quest(18119) == 0 && isbegin_quest(18085) == 2){
		if(BaseLevel < 200) {
			mes "[Grey Wolf Villager]";
			mes "Maram should be here soon.";
			mes "I wonder where the others are...";
			close;
		}
		mes "[Grey Wolf Villager]";
		mes "You, can you wait for a bit?";
		mes "Maram was looking for you.";
		mes "Wait here for a while. He'll be here soon.";
		next;
		mes "[Grey Wolf Villager]";
		mes "Come to think of it, did you gave him the doll that you've been carrying on your head for a while?";
		mes "He was talking to a doll that looked like that.";
		next;
		mes "[Grey Wolf Villager]";
		mes "Do you think Maram was so busy that he's relieving stress?";
		mes "This reminds me, were you talking to that doll too?";
		next;
		cloaknpc("Maram#ep19maram01",false,getcharid(0));
		npctalk "Maram: Have you waited long?","Maram#ep19maram01",bc_self;
		mes "[Grey Wolf Villager]";
		mes "Maram is here.";
		mes "Then, let's share our conversation.";
		next;
		npctalk "Maram: Thanks!","Maram#ep19maram01",bc_self;
		mes "[Maram]";
		mes "Have you been waiting for a while?";
		cutin "ep18_maram_01.png",2;
		next;
		select("We're you talking to the doll?");
		mes "[Maram]";
		mes "Ah, yes. You're right, I was talking to the doll after the result. It's true.";
		mes "Ellie, was it?";
		cutin "ep18_maram_02.png",2;
		next;
	OnEvent:
		cloaknpc("Miriam#ep19miriam01",false,getcharid(0));
		cloaknpc("Suad#ep19suad01",false,getcharid(0));
		npctalk "Suad: Fortunately, we didn't cross roads.","Suad#ep19suad01",bc_self;
		mes "[Miriam]";
		mes "What's the reason you're all here together, Maram?";
		cutin "ep18_miriam_01.png",0;
		next;
		mes "[Suad]";
		mes "I'm sorry but I don't have much time. I have to go to Niren. Because of the problems with the parliament.";
		cutin "ep18_suad_01.png",0;
		next;
		mes "[Maram]";
		mes "Now that we're all here, I'll explain everything detail by detail.";
		mes "First of all, I called for the adventurer because Ellie came...";
		cutin "ep18_maram_01.png",2;
		next;
		mes "[Maram]";
		mes "She came to Rachel without any fear on her Mini Ellie form.";
		mes "She had a local kid that she met on the way to the temple as her guide.";
		next;
		mes "[Maram]";
		mes "Ellie went to north where Bagot passed through, it's the place where Jormungandr was sealed.";
		mes "She said they figured out their route.";
		next;
		mes "[Maram]";
		mes "She asked me to come all the way here to tell the adventurer the news.";
		next;
		mes "[Miriam]";
		mes "So, can we find the Fragment of Ymir's Heart?";
		mes "Where's Ellie now?";
		cutin "ep18_miriam_01.png",0;
		next;
		mes "[Maram]";
		mes "Like what happened to the adventurer last time, the connection was cut off and it transformed to something like a cotton ball.";
		mes "I was concern at first. So I brought it to my room....";
		cutin "ep18_maram_01.png",2;
		next;
		mes "[Suad]";
		mes "Then, where did she tell you to go?";
		mes "We should go north, find the Fragment of Ymir's Heart and punish that man named Bagot.";
		cutin "ep18_suad_01.png",0;
		next;
		mes "[Miriam]";
		mes "Stay here Suad.";
		mes "I'll go instead.";
		mes "Maram, can you be the one to contact in the middle?";
		cutin "ep18_miriam_01.png",0;
		next;
		mes "[Suad]";
		mes "..... I don't like it, but there's no helping it.";
		mes "This is an important time for the Grey Wolf Village and the natives.";
		cutin "ep18_suad_01.png",0;
		next;
		mes "[Suad]";
		mes "Even if a parliament was formed, things won't change overnight.";
		mes "For the time being, there should be a person who coordinate everything from the middle as if it's natural.";
		next;
		cutin "ep18_maram_01.png",2;
		mes "[Maram]";
		mes "Yes, that's why my mother trusts you and she's doing her best to work on it.";
		mes "I'll keep you informed, just wait and trust Miriam and the adventurer.";
		next;
		mes "[Suad]";
		mes "I can't help it.";
		mes "I'd love to talk more, but I have to go to the temple.";
		mes "I'm counting on you.";
		cutin "ep18_suad_01.png",0;
		next;
		mes "[Miriam]";
		mes "So, where do we go?";
		mes "Do you know where, adventurer?";
		cutin "ep18_miriam_01.png",0;
		next;
		cutin "ep18_maram_01.png",2;
		mes "[Maram]";
		mes "She told me to come to the main building of the Varmundt's mansion.";
		mes "Can you lead the way?";
		setquest 18119;
		close3;
	}
	mes "[Grey Wolf Villager]";
	mes "I've been hearing a lot about you lately.";
	mes "Thank you for doing it for my village.";
	close2;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(18085) == 2 && isbegin_quest(18119) == 0 && BaseLevel >= 200";
end;
}

wolfvill,220,85,3	script	Maram#ep19maram01	4_EP18_MARAM,{
	if(isbegin_quest(18119) == 0){
		cutin "ep18_maram_01.png",2;
		mes "[Maram]";
		mes "Have you been waiting for a while?";
		mes "Miriam and Suad will be here soon.";
		mes "Oh, it seems they have arrived.";
		next;
		doevent "Grey Wolf Villager#ep19::OnEvent";
	}
	cutin "ep18_maram_01.png",2;
	mes "[Maram]";
	mes "Shall we go?";
	mes "I'm looking forward to Varmundt's Mansion.";
	mes "You can leave first, I'll follow you!";
	close3;
}

wolfvill,213,84,5	script	Suad#ep19suad01	4_EP18_SUAD,{
	if(isbegin_quest(18119) == 0){
		cutin "ep18_suad_01.png",0;
		mes "[Suad]";
		mes "I have a lot of things I want to say, but let's listen to Maram for now.";
		close3;
	}
	cutin "ep18_suad_01.png",0;
	mes "[Suad]";
	mes "Sigh.... Why can't we just do these important things together....";
	mes "Sorry, I can't help it.";
	close3;
}

wolfvill,216,85,5	script	Miriam#ep19miriam01	4_EP18_MIRIAM,{
	if(isbegin_quest(18119) == 0){
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "Let's listen to Maram first.";
		mes "Calling us here, it must be something important.";
		close3;
	}
	cutin "ep18_miriam_01.png",0;
	mes "[Miriam]";
	mes "I'm on your care for this journey.";
	mes "I look forward to working with you.";
	close3;
}

//= BA_IN01
ba_in01,375,102,0	script	#ep19_evt01	HIDDEN_WARP_NPC,3,3,{
	end;
	
OnTouch:
	if(isbegin_quest(18119) > 0 && isbegin_quest(18120) < 2){
		cloaknpc("Smart Ellie#ep19elly01",false,getcharid(0));
		cloaknpc("Lazy#ep19lazy01",false,getcharid(0));
		cloaknpc("Crux#ep19crux01",false,getcharid(0));
		cloaknpc("Miriam#ep19miriam02",false,getcharid(0));
		cloaknpc("Maram#ep19maram02",false,getcharid(0));
	}
end;
}

ba_in01,384,114,4	script	Smart Ellie#ep19elly01	4_EP17_TABLET,{
	if(isbegin_quest(18119) == 1){
		cutin "ep172_beta.png",2;
		mes "[Ellie]";
		mes "Welcome, guest.";
		mes "The others are already here.";
		mes "I explained the situation to them in advance.";
		next;
		cutin "ep16_crux_findel02.png",1;
		mes "[Crux]";
		mes "Long time no see.";
		mes "I came here for the regular report and heard about what happened.";
		next;
		cutin "16lei_01.png",0;
		mes "[Lazy]";
		mes "I heard that something big happened in Rachel?";
		mes "I also just stopped by for the regular report and heard some amazing stories.";
		next;
		mes "[Lazy]";
		mes "I met the valuable guests from Rachel, I only came here because Tess insisted that I come in his stead, I think I did well~";
		next;
		mes "[Lazy]";
		mes "You two, have you been to Yuno before going into Schwarzwald?";
		mes "No? You dropped by while transferring? Then, go one more time. No, go twice!";
		next;
		mes "[Maram]";
		mes "Haha! Lazy? You're a very funny person!";
		mes "Sure. Can you tell me about Yuno's good restaurants later?";
		mes "I think I'm going to stay a lot from now on, please take care of me.";
		cutin "ep18_maram_02.png",2;
		next;
		mes "[Miriam]";
		mes "Thank you so much for your care.";
		mes "I will definitely visit Yuno after work.";
		cutin "ep18_miriam_01.png",0;
		next;
		mes "[Miriam]";
		mes "Anyway, I know that Bagot has more colleagues that cooperates with him.";
		mes "It's according to the information that I got out from my father.";
		next;
		mes "[Miriam]";
		mes "Maybe Bagot had left to join his colleagues....";
		next;
		cutin "16lei_01.png",1;
		mes "[Lazy]";
		mes "You mentioned it was north, something about Isgard, correct?";
		mes "Do you mean they're over there?";
		next;
		cutin "ep172_beta.png",2;
		mes "[Ellie]";
		mes "Exactly right.";
		mes "He mentioned the Legend of Jormungandr, the only northern continent that is associated with Jormungandr is Isgard.";
		next;
		mes "[Ellie]";
		mes "And the Master collection No.3 that they took, is an aircraft with a record of a voyage to Isgard.";
		next;
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "So Bagot had Isgard in mind before he even talk to my father?";
		next;
		cutin "ep172_beta.png",2;
		mes "[Ellie]";
		mes "Most likely, New guest.";
		mes "It was presumed that the Illusions discovered a route stored in the airship and made it as their target candidate.";
		next;
		mes "[Ellie]";
		mes "While the guests were active in Arunafeltz, the airship was quite busy.";
		mes "It traveled all over the continent of Midgard including Arunafeltz and Schwarzwald, and it always left a trail toward north after.";
		next;
		mes "[Ellie]";
		mes "Wile(1), is the name of Master collection No. 3.";
		mes "They managed to decrypt Wile(1) and they disconnected it from the core.";
		close2;
		cloaknpc("Navigator Ginger#ep19gg02",false,getcharid(0));
		npctalk "Ginger: Reporting. Everything is normal!","Navigator Ginger#ep19gg02",bc_self;
		sleep2 1000;
		mes "[Ellie]";
		mes "You've arrived just in time.";
		mes "Ginger will tell us about the flight routes.";
		cutin "ep172_beta.png",2;
		next;
		mes "[Ginger]";
		mes "Eh, The report, Everything is normal!";
		mes "I have checked the wall in Isgard.";
		mes "Entry is not possible.";
		cutin "ep172_beta.png",0;
		next;
		mes "[Ginger]";
		mes "I have found a high flow of energy near the barrier. And traces of Wile(1).";
		mes "But how did the master come and go using Wile(1)?";
		next;
		cutin "16lei_01.png",1;
		mes "[Lazy]";
		mes "Varmundt is a great sage. Wouldn't he have used a special method?";
		mes "The power of the Fragment of Ymir's Heart for example.";
		next;
		cutin "ep16_crux_findel02.png",0;
		mes "[Crux]";
		mes "I apologize for interrupting, but if it's related to Jormungandr, wouldn't it be better to ask the members of royal family for help?";
		next;
		cutin "16lei_01.png",1;
		mes "[Lazy]";
		mes "Rune-Midgarts? Why?";
		next;
		cutin "ep16_crux_findel02.png",0;
		mes "[Crux]";
		mes "Ff you know the secret of the establishing of Rune-Midgarts, the true nature of the story is inextricably linked to Jormungandr.";
		next;
		mes "[Crux]";
		mes "I know that the Gaebolgs are heading north to research Jormungandr";
		mes "I wonder if the that place is the Isgard that you're talking about...";
		next;
		mes "[Maram]";
		mes "The curse of Jormungandr!";
		mes "Surely the Gaebolgs have a lof of information abot Jormungandr.";
		cutin "ep18_maram_01.png",2;
		next;
		cutin "ep172_beta.png",0;
		mes "[Ellie]";
		mes "You mention that they're researching on the northern, right?";
		mes "Does that mean that they're free to come and go beyond the barrier?";
		next;
		mes "[Ellie]";
		mes "Humans, we've established the course and all that's left is to clear the path towards the barrier.";
		mes "Go ahead and find a way to do it.";
		mes "Ginger and I will maintain the airship.";
		next;
		cutin "16lei_01.png",1;
		mes "[Lazy]";
		mes "Okay. This is not the time for long discussions.";
		mes "Hey, You should have mentioned that earlier, Crux!";
		next;
		cutin "ep16_crux_findel02.png",0;
		mes "[Crux]";
		mes "Well... An affair related to the Royal family should not simply be discussed around.";
		mes "Please explain it to them well so that I won't get scolded later.";
		next;
		cutin "16lei_01.png",1;
		mes "[Lazy]";
		mes "Let's do it like this.";
		mes strcharinfo(0) + ", go to Prontera with Crux.";
		mes "Ellie and Giner will be the one in charge of the airship, and I will prepare for the trip with the people from Rachel.";
		next;
		mes "[Maram]";
		mes "That would be nice.";
		mes "We can't easily enter the Kingdom of Rune-Midgarts.";
		cutin "ep18_maram_01.png",2;
		next;
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "I will be learning a lot while preparing here.";
		mes "See you later.";
		next;
		cutin "ep172_beta.png",2;
		mes "[Ellie]";
		mes "Well, then Ginger and I should start fixing the airship, yes?";
		next;
		cutin "ep16_crux_findel02.png",0;
		mes "[Crux]";
		mes "I'll take you to Prontera.";
		mes "Let me know when you're ready.";
		completequest 18119;
		setquest 18120;
		questinfo_refresh;
		close2;
		cutin "ep16_crux_findel02.png",255;
		end;
	}
	if(isbegin_quest(18120) == 1){
		cutin "ep172_beta.png",2;
		mes "[Ellie]";
		mes "Then guest, do your own thing.";
		mes "We're going to be determining the route that we're going to use.";
		close3;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(18119) == 1";
end;
}


ba_in01,386,110,3	script	Crux#ep19crux01	4_EP16_CRUX,{
	if(isbegin_quest(18119) == 1){
		cutin "ep16_crux_findel02.png",1;
		mes "[Crux]";
		mes "Long time no see.";
		mes "My lady Spica and Skia sends their regards.";
		mes "I'm glad to see that you're still the same.";
		close3;
	}
	if(isbegin_quest(18120) == 1 || isbegin_quest(18121) == 1){
		cutin "ep16_crux_findel02.png",1;
		mes "[Crux]";
		mes "Then, Shall I take you to the Prontera Castle?";
		next;
		if(select("Let's go right now!:Wait for a while.") == 2){
			mes "[Crux]";
			mes "Let me know when you are ready.";
			close3;
		}
		mes "[Crux]";
		mes "Alright. I will take you to the royal palace immediately.";
		close2;
		cutin "",255;
		warp "prt_cas",147,338;
		end;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(18120) == 1";
end;
}

ba_in01,386,113,3	script	Lazy#ep19lazy01	4_M_LAZY,{
	if(isbegin_quest(18119) == 1){
		cutin "16lei_01.png",2;
		mes "[Lazy]";
		mes "How was Rachel?";
		mes "Yuno is the best, right?";
		close3;
	}
	if(isbegin_quest(18120) == 1){
		cutin "16lei_01.png",2;
		mes "[Lazy]";
		mes "Is it true that we have to cross mountains and the sea?";
		mes "So we're talking about a place farther than Lutie, it must be really cold, right?";
		next;
		mes "[Lazy]";
		mes "I'm going to do some shopping while you're at Prontera.";
		mes "I will go to Lighthalzen's department store with the people from Rachel.";
		close3;
	}
	end;
}

ba_in01,386,107,3	script	Miriam#ep19miriam02	4_EP18_MIRIAM,{
	if(isbegin_quest(18119) == 1){
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "You can tell how amazing Varmundt is.";
		mes "You can tell by just looking at this magnificent mansion.";
		close3;
	}
	if(isbegin_quest(18120) == 1){
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "I will contact Suad to check future schedules while you're in Prontera.";
		mes "We can prepare the necessary items with Lazy's advice, right?";
		close3;
	}
	end;
}

ba_in01,383,104,1	script	Maram#ep19maram02	4_EP18_MARAM,{
	if(isbegin_quest(18119) == 1){
		cutin "ep18_maram_01.png",2;
		mes "[Maram]";
		mes "In my lifetime I've been into many places, but never to a place like this before.";
		mes "What in the world....";
		mes "This is the reason why Ellie likes to brag about her master. Hahaha.";
		close3;
	}
	if(isbegin_quest(18120) == 1){
		cutin "ep18_maram_01.png",2;
		mes "[Maram]";
		mes "I will do my best to prepare for the trip.";
		mes "See you!";
		close3;
	}
	end;
}

ba_in01,380,111,7	script	Navigator Ginger#ep19gg02	4_EP17_TABLET,{
	cutin "ep172_beta.png",2;
	mes "[Ginger]";
	mes "I will analyze once more when you are near the barrier.";
	mes "By simulating a few more of the predicted routes, a stable route might be determined.";
	next;
	mes "[Ginger]";
	mes "Ginger is busy~!";
	close3;
}

//= PRT_CAS
prt_cas,147,337,0	script	#ep19_evt02	HIDDEN_WARP_NPC,3,3,{
	end;
	
OnTouch:
	if(isbegin_quest(18120) == 1){
		cutin "ep16_crux_findel02.png",2;
		mes "[Crux]";
		mes "I already requested for an audience.";
		mes "Let's go inside to join the meal.";
		close2;
		cutin "",255;
		navigateto("prt_cas",320,270);
	}
end;
}

prt_cas,309,287,0	script	#ep19_evt03	HIDDEN_WARP_NPC,3,3,{
	end;
	
OnTouch:
	if(isbegin_quest(18120) == 1){
		cloaknpc("Crux#ep19crux02",false,getcharid(0));
		cloaknpc("Catherine#ep19c01",false,getcharid(0));
	}
end;
}

prt_cas,320,270,4	script	Crux#ep19crux02	4_EP16_CRUX,{
	if(isbegin_quest(18120) == 1){
		cutin "ep16_crux_findel02.png",1;
		mes "[Crux]";
		mes "As I mentioned before, I brought " + strcharinfo(0) + " here since they are looking for information about where Jormungandr is sealed.";
		next;
		cutin "ep171_nihil01.png",2;
		mes "[Nihil]";
		mes "The day has come when I can be of help to you.";
		mes "Strictly speaking, it's a secret of the Gaebolg family, everyone in the royal family is tangled with the affair.";
		mes "The seven royal families of Rune-Midgarts are all responsible for that";
		next;
		cutin "ep16gao_1.bmp",0;
		mes "[Catherine]";
		mes "I heard that you're looking for the lands sealed by Jormungandr.";
		mes "Do you really want to go to that land?";
		next;
		select("We must go!");
		mes "[Catherine]";
		mes "It is known that Gaebolg family were cursed by Jormungandr.";
		mes "Everyone in this room knows it.";
		next;
		mes "[Catherine]";
		mes "Thank to this, the Gaebolgs are the most related thing to Jormungandr in the continent of Midgard.";
		mes "That is why we know other things that is not known to the world.";
		next;
		mes "[Catherine]";
		mes "When the cult of Jormungandr disappeared from this land, not everyone died and disappeared.";
		next;
		mes "[Catherine]";
		mes "They were trapped in a land of ice beyond the sea, and that land was sealed by an unknown forces.";
		mes "And there are guardians who protects the seal.";
		next;
		select("Guardians?!");
		cutin "ep171_nihil01.png",2;
		mes "[Nihil]";
		mes "Are you surprised? After I was officially crowned, I learned about it from a remark in the royal palace.";
		next;
		mes "[Nihil]";
		mes "What's even more surprising is, Jormungandr was sealed in the Land of Ice, Isgard.";
		next;
		mes "[Nihil]";
		mes "It is told that the members of the Gaebolg family were traveling back and forth to Isgard before.";
		mes "That is why I have called for Queen Catherine here today.";
		next;
		cutin "ep16gao_1.bmp",0;
		mes "[Catherine]";
		mes "That is right.";
		mes "Tristan III has 3 sisters.";
		mes "Two of them are currently in Isgard.";
		next;
		select("How? No, why?");
		mes "[Catherine]";
		mes "Of course, it's to break the curse of Jormungandr.";
		mes "The reason why Tristan III became to what he is now is because of the curse.";
		mes "It happened after he tried to chase after the curse of Jormungandr.";
		next;
		mes "[Catherine]";
		mes "It is sad to see the death of a blood relative.";
		mes "It is not strange to see the other princesses set out to break the curse.";
		next;
		mes "[Catherine]";
		mes "My nephew will be here soon, listen to his story.";
		mes "He is a kid who actually come and go to Isgard, so he will know how to get there.";
		next;
		cutin "ep171_nihil01.png",2;
		mes "[Nihil]";
		mes "I had a messaged pass to him to come straight here.";
		mes "He'll be here in a moment.";
		next;
		cutin "ep16gao_1.bmp",0;
		mes "[Catherine]";
		mes "...I wish I had known about that box earlier.";
		next;
		cloaknpc("Lehar#ep19lehar01",false,getcharid(0));
		mes "[Lehar]";
		mes "You called for me?";
		mes "Oh, my aunt is here too. Why is Crux also here?";
		cutin "ep19_lehar02.png",1;
		next;
		cutin "ep16_crux_findel02.png",0;
		mes "[Crux]";
		mes "Long time no see.";
		mes "I'm glad to see you in good health.";
		mes "Everyone waited for some time.";
		next;
		cutin "ep19_lehar01.png",1;
		mes "[Lehar]";
		mes "Oh, yes. Yes.";
		mes "Brother Nihil, your highness? Your majesty?";
		mes "I'm still not used to this.";
		mes "Anyway, why did you call for me?";
		mes "I was a little late because I was running errands for my aunt.";
		npctalk "Catherine: Lehar, your court manners are surprisingly the same from when you were young.","Catherine#ep19c01",bc_self;
		npctalk "Lehar: I apologize...","Lehar#ep19lehar01",bc_self;
		next;
		cutin "ep171_nihil02.png",2;
		mes "[Nihil]";
		mes "To put it bluntly, are you leaving for Isgard soon?";
		mes "If you do, I want you to take this person and " + (Sex ? "his" : "her") + " colleagues with you." ;
		next;
		cutin "ep19_lehar05.png",1;
		mes "[Lehar]";
		mes "Huh? Why so sudden? Why, what's going on?";
		mes "Wasn't it a secret???";
		mes "Was it okay to say it?!!!!";
		next;
		cutin "ep16gao_1.bmp",0;
		mes "[Catherine]";
		mes "Lehar. Calm down.";
		mes "This person must go to Isgard for a reason.";
		next;
		select("Tell the story of the Illusions.");
		cutin "ep19_lehar01.png",1;
		mes "[Lehar]";
		mes "If so, then it's a big deal.";
		mes "I'll have to use the Guardians Box.";
		mes "Please allow it.";
		next;
		cutin "ep171_nihil01.png",2;
		mes "[Nihil]";
		mes "Sure. " + strcharinfo(0) + ", let's move to the next room.";
		mes "The Guardians Box is there.";
		next;
		cutin "ep16gao_1.bmp",0;
		mes "[Catherine]";
		mes "I will be on my way.";
		mes "I hope you achieve what you want.";
		mes "Lehar, say hello to your mother for me.";
		npctalk "Lehar: I will! My mother is always at peace.","Lehar#ep19lehar01",bc_self;
		next;
		cutin "ep16_crux_findel02.png",0;
		mes "[Crux]";
		mes "I will escort the queen.";
		mes "I look forward to seeing you next time.";
		next;
		cloaknpc("Crux#ep19crux02",true,getcharid(0));
		cloaknpc("Catherine#ep19c01",true,getcharid(0));
		cutin "ep19_lehar01.png",1;
		mes "[Lehar]";
		mes "Shall we go? The time is just right.";
		completequest 18120;
		setquest 18121;
		close2;
		cutin "",255;
		navigateto("prt_cas",331,343);
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(18120) == 1";
end;
}

prt_cas,323,266,4	duplicate(dummynpc2)	Catherine#ep19c01	4_EP16_EGEO

prt_cas,317,266,6	script	Lehar#ep19lehar01	4_EP19_LEHAR,{
	if(isbegin_quest(18121) == 1){
		cutin "ep19_lehar01.png",0;
		mes "[Lehar]";
		mes "Let us go.";
		mes "To brother Nihil's, Ah no, I mean his majesty's private room.";
		next;
		cutin "ep171_nihil02.png",2;
		mes "[Nihil]";
		mes "You can just call me brother when we're together.";
		mes "You're not tied to the palace rules anyway...";
		next;
		cutin "ep19_lehar07.png",0;
		mes "[Lehar]";
		mes "Oh, but please don't tell my mother.";
		mes "I really hate studying etiquette!";
		npctalk "Lehar: I would rather perform an exorcism in a monastery.","",bc_self;
		close2;
		cutin "",255;
		navigateto("prt_cas",331,343);
		end;
	}
	end;
}

prt_cas,322,336,0	script	#ep19_evt04	HIDDEN_WARP_NPC,3,3,{
	end;
	
OnTouch:
	if(isbegin_quest(18121) == 1){
		cloaknpc("Lehar#ep19lehar02",false,getcharid(0));
		cloaknpc("Nihil#ep19nh01",false,getcharid(0));
	}
end;
}

prt_cas,331,343,4	script	Nihil#ep19nh01	4_EP17_NIHIL_K,{
	if(isbegin_quest(18121) == 1){
		mes "[Nihil]";
		mes "This is a Guardian Box, it looks like a normal book.";
		mes "Opening the cover the box-shaped space inside.";
		next;
		mes "[Nihil]";
		mes "Only direct descendants of the seven royal family can use it.";
		mes "In this case, we'll need my seal and from Lehar.";
		next;
		cutin "ep19_lehar02.png",2;
		mes "[Lehar]";
		mes "You need to get an ^4D4DFFinvitation^000000 to enter Isgard.";
		mes "Adventurer, how many colleagues are going with you?";
		next;
		select("I have a lot of colleagues.");
		cutin "ep19_lehar01.png",2;
		mes "[Lehar]";
		mes "Well... it would be too much by land.";
		mes "We have to cross the sea, are you planning on taking a boat?";
		next;
		select("We're going to ride Varmundt's airship.");
		mes "[Lehar]";
		mes "Oh! An airship! That's great.";
		mes "Then, I will send a request to get permission for your entry.";
		next;
		cutin "",255;
		mes "Lehar began to write something on a fine paper with the royal seal.";
		mes "From time to time, Nihil is correcting the expressions he's writing.";
		next;
		cutin "ep19_lehar01.png",2;
		mes "[Lehar]";
		mes "Ah, it's done!";
		mes "Then, I'll stamp the seal and put the letter in the box.";
		mes "Now we just have to wait.";
		next;
		cutin "ep171_nihil01.png",0;
		mes "[Nihil]";
		mes "I hope they find it in time.";
		next;
		select("What is this?");
		mes "[Nihil]";
		mes "This is a box that was left behind by Tristan Gaebolg I.";
		mes "It's a relic that was kept in the royal treasury as one of his treasured possessions.";
		next;
		mes "[Nihil]";
		mes "According to the records, it is a letter box that was used by the founding king.";
		mes "Some of his subjects tried to write letters and store it inside since they believed that it was just a box for letters, nothing happened that time.";
		next;
		cutin "ep19_lehar01.png",2;
		mes "[Lehar]";
		mes "That's why in the records it was named as the letter box.";
		mes "It was actually a magical relic that only responses to certain people's mana.";
		next;
		mes "[Lehar]";
		mes "My aunt who was organizing the library records and found out about the existence of this box, so she tried putting a letter in it.";
		next;
		select("Did it work?");
		mes "[Lehar]";
		mes "It did.";
		mes "The people who replied were the Guardians of Isgard.";
		mes "If you have an invitation with their blessing, you'll be able to go in and out.";
		next;
		specialeffect  EF_BEGINSPELL3,AREA,"Book-shaped Box#ep19box0";
		specialeffect  EF_SPHERE,AREA,"Book-shaped Box#ep19box0";
		cutin "ep171_nihil01.png",0;
		mes "[Nihil]";
		mes "It looks like we got a reply.";
		next;
		cutin "ep19_lehar02.png",2;
		mes "[Lehar]";
		mes "It's an invitation! Oh, it came with another letter.";
		mes "It says welcome.";
		mes "They say it's been a while since Varmundt Airship last came, it has been there before?";
		next;
		cutin "ep19_lehar02.png",2;
		mes "[Lehar]";
		mes "Alright. With this, you and your colleagues can go to Isgard.";
		next;
		cutin "ep19_lehar01.png",2;
		mes "[Lehar]";
		mes "They also said that we can leave it everything to the one who manages the airship.";
		next;
		cutin "ep171_nihil01.png",0;
		mes "[Nihil]";
		mes "It looks like everything worked out.";
		next;
		cutin "ep19_lehar01.png",2;
		mes "[Lehar]";
		mes "Can I board the airship too?";
		mes "I will stop by the monastery and the cathedral before going.";
		next;
		cutin "ep171_nihil01.png",0;
		mes "[Nihil]";
		mes "Lehar, ask Crux to guide you to Varmundt's Mansion.";
		next;
		mes "[Nihil]";
		mes "It will be better if Lehar join because you will need a guide in Isgard.";
		mes "Are you going right away?";
		completequest 18121;
		setquest 18122;
		getitem 1000607,1;
		next;
		if(select("Return to Varmundt Mansion.:I'm going to stop by Prontera.") == 1){
			cutin "ep19_lehar01.png",2;
			mes "[Lehar]";
			mes "Then, I will go to Varmundt's mansion on my own, so let's ride the airship together!";
			mes "I'm looking forward to it.";
			close2;
			cutin "",255;
			warp "ba_maison",187,251;
		} else {
			cutin "ep19_lehar01.png",2;
			mes "[Lehar]";
			mes "Then let's head out together.";
			mes "I have to go to the cathedral.";
			mes "I'll see you later at Varmundt's Mansion!";
			close2;
			cutin "",255;
			warp "prontera",155,320;
		}
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(18121) == 1";
end;
}

prt_cas,334,342,3	duplicate(dummynpc2)	Lehar#ep19lehar02	4_EP19_LEHAR
prt_cas,331,342,3	duplicate(dummynpc2)	Book-shaped Box#ep19box0	4_POINT_WHITE

ba_maison,186,251,0	script	#ep19_evt05	HIDDEN_WARP_NPC,3,3,{
	end;
	
OnTouch:
	if(isbegin_quest(18122) == 1){
		cutin "ep172_beta.png",2;
		mes "[Ginger]";
		mes "I'm glad we me just in time.";
		mes "I finished the preparation for the airship while waiting for the other guests.";
		mes "I'm waiting for the other guests to arrive to guide them to the airship boarding area.";
		next;
		mes "[Ginger]";
		mes "A guest named Lehar also came and asked me to guide him, I'll guide him.";
		mes "Guest, Do you know anything about him?";
		next;
		select "Lehar is our new companion.";
		mes "[Ginger]";
		mes "I see. Then you don't have to worry, I will guide him well.";
		next;
		mes "[Ginger]";
		mes "The other customers are coming.";
		next;
		cutin "16lei_01.png",0;
		mes "[Lazy]";
		mes "Hey, Did you wait long?";
		mes "No, right? We came right in time, right?";
		next;
		cutin "ep18_maram_01.png",2;
		mes "[Maram]";
		mes "I've brought the news to Suad and mother.";
		mes "We're all set for the departure.";
		next;
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "We can leave anytime.";
		mes "You said you'd go to Prontera to get information on how to get to Isgard, how did it go?";
		next;
		select "Show the Guardian's Invitation."	;
		cutin "16lei_01.png",0;
		mes "[Lazy]";
		mes "So that's our ticket in?";
		mes "It's like magic, it's very convenient.";
		mes "So, are we leaving now?";
		next;
		select "There's more company.";
		cutin "ep19_lehar01.png",1;
		mes "[Lehar]";
		mes strcharinfo(0) + "!";
		mes "Am I late?";
		mes "Is that them?";
		mes "These are the colleagues you mentioned earlier!";
		next;
		mes "[Lehar]";
		mes "Hello.";
		mes "I'm Lehar, the one who'll guide you to Isgard.";
		mes "I look forward to working with you.";
		next;
		cutin "ep172_beta.png",2;
		mes "[Ginger]";
		mes "Guests, shall we go?";
		mes "Please hurry and make a decision!";
		next;
		cutin "16lei_01.png",0;
		mes "[Lazy]";
		mes "Why are you in such a hurry?";
		mes "We haven't even said our introductions yet.";
		mes "My name is Lazy.";
		mes "Have you ever been to Yuno?";
		next;
		cutin "ep19_lehar01.png",2;
		mes "[Lehar]";
		mes "What? Of course. I transferred from Yuno.";
		mes "First, uhm.. let me check the passengers for a bit.";
		next;
		mes "[Lehar]";
		mes "Oh, there's the woman with the long hair and veil.";
		mes "We're all here, right?";
		mes "I think we're set to go. Where should we go?";
		next;
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes ".....?";
		mes "Do you know me?";
		next;
		cutin "ep19_lehar02.png",2;
		mes "[Lehar]";
		mes "No? Of course not.";
		mes "Nice to meet you!";
		next;
		cutin "ep172_beta.png",1;
		mes "[Ginger]";
		mes "Then guests, please move to the airship boarding area.";
		mes "Ginger will take you on a special express~!";
		completequest 18122;
		setquest 18123;
		navigateto("ba_in01",30,264);
		close3;
	}
end;

OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(18122) == 1";
end;
}

ba_in01,30,264,4	script	Navigator Ginger#ep19gg01	4_EP17_TABLET,{
	if(isbegin_quest(18123) > 0){
		mes "[Ginger]";
		mes "We are going to Isgard if you're ready.";
		mes "Are you going to board right now?";
		next;
		if(select("Board the airship!:Give me a minute.") == 2){
			mes "[Ginger]";
			mes "Let us go when you're ready.";
			close3;
		}
		mes "[Ginger]";
		mes "This way guest.";
		close2;
		cutin "",255;
		warp "air_if",53,71;
		end;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(18123) == 1";
end;
}

air_if,54,68,0	script	#ep19_evt06	HIDDEN_WARP_NPC,7,7,{
	end;
	
OnTouch:
	if(isbegin_quest(18123) > 0 && isbegin_quest(18124) == 0){
		cloaknpc("Lehar#ep19lehar03",false,getcharid(0));
		cloaknpc("Lazy#ep19lazy02",false,getcharid(0));
		cloaknpc("Maram#ep19maram03",false,getcharid(0));
		cloaknpc("Miriam#ep19miriam03",false,getcharid(0));
	}
end;
}

air_if,35,60,0	duplicate(#ep19_evt06)	#ep19_evt06_1	HIDDEN_WARP_NPC,5,5

air_if,46,71,4	script	Navigator Ginger#ep19gg03	4_EP17_TABLET,{
	if(isbegin_quest(18123) == 1){
		cutin "ep172_beta.png",2;
		mes "[Ginger]";
		mes "We are going to fly to the final point where our test flight was conducted.";
		mes "Also, where is the way to cross the barrier?";
		next;
		cutin "ep19_lehar01.png",0;
		mes "[Ginger]";
		mes "Didn't I gave you an invitation to visit the Ice Castle?";
		mes "You need to give that invitation to the one managing the airship...";
		next;
		select "Give the invitation to Ginger."	;
		cutin "ep172_beta.png",2;
		mes "[Ginger]";
		mes "Ah! It's a puzzle piece!";
		mes "A specific wavelength code is detected in the invitation..";
		next;
		mes "[Ginger]";
		mes "Yes. I know what to do with it.";
		mes "The guests can rest until we arrive.";
		delitem 1000607,1;
		completequest 18123;
		setquest 18129;
		questinfo_refresh;
		close3;
	}
	if(isbegin_quest(18129) == 1){
		cutin "ep172_beta.png",2;
		mes "[Ginger]";
		mes "We are flying on the best route.";
		mes "The guests can rest until we arrive at the destination.";
		close3;
	}
	if(isbegin_quest(18124) == 1){
		cutin "ep172_beta.png",2;
		mes "[Ginger]";
		mes "Guest, This way.";
		mes "I will stay here to protect the airship.";
		mes "Feel free to ask if you want to return to the Varmundt Mansion.";
		next;
		if(select("Get off now.:Get off later.") == 2){
			mes "[Ginger]";
			mes "Okay.";
			mes "Tell me later.";
			close3;
		}
		mes "[Ginger]";
		mes "Then *Shh*~";
		close2;
		cutin "",255;
		warp "jor_tail",219,53;
		end;
	}
	cutin "ep172_beta.png",2;
	mes "[Ginger]";
	mes "The Airship is always operational.";
	mes "Where should I take you?";
	next;
	if(select("Varmundt Mansion:Isgard") == 1){
		mes "[Ginger]";
		mes "Then *Swoosh*~";
		close2;
		cutin "",255;
		warp "ba_in01",30,262;
	} else {
		mes "[Ginger]";
		mes "Then *Shh*~";
		close2;
		cutin "",255;
		warp "jor_tail",219,53;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(18123) == 1";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(18124) == 1";
end;
}

air_if,32,63,3	script	Lehar#ep19lehar03	4_EP19_LEHAR,{
	if(isbegin_quest(18123) == 1){
		cutin "ep19_lehar01.png",2;
		mes "[Lehar]";
		mes "Give the invitation from the royal palace to Ginger.";
		mes "She will be able to make it so that the entire airship will be recognize as one authorized entity.";
		close3;
	}
	if(isbegin_quest(18129) == 1){
		cutin "ep19_lehar01.png",2;
		mes "[Lehar]";
		mes "I didn't introduce myself properly since we were in a hurry to board, right?";
		mes "I'm Lehar from the Saint Capitolina Monastery.";
		next;
		mes "[Lehar]";
		mes "I just happened to be going to Isgard too, and I'll be there to help you with your research on Jormungandr.";
		next;
		cutin "ep19_leizi01.png",0;
		mes "[Lazy]";
		mes "You're bright, you're like that young man, Rookie!";
		mes "I like you.";
		mes "Feel free to call me Lazy.";
		next;
		cutin "ep18_maram_01.png",2;
		mes "[Maram]";
		mes "I'm Maram. I'm also here to help them on this journey.";
		mes "I will surely do well as long it's about delivering supplies or news.";
		next;
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "I'm Miriam.";
		mes "I came here with Maram.";
		mes "I'm looking forward to working with you.";
		next;
		cutin "ep19_lehar02.png",2;
		mes "[Lehar]";
		mes "Miriam?";
		mes "Oh, how mysterious~";
		mes "I'll pretend I didn't hear that.";
		next;
		cutin "ep18_miriam_03.png",0;
		mes "[Miriam]";
		mes "Hm?";
		mes "Is that so...";
		next;
		cutin "ep19_leizi01.png",0;
		mes "[Lazy]";
		mes "Hey, young man.";
		mes "Judging by your outfit, are you from the exorcism family?";
		next;
		cutin "ep19_lehar01.png",2;
		mes "[Lehar]";
		mes "What? Oh, yes. That's right!";
		mes "I've been living in the Capitolina Monastery since I was a kid...";
		next;
		cutin "ep19_leizi01.png",0;
		mes "[Lazy]";
		mes "So what's your connection with the Gaebolg Family?";
		mes "Oh, are you a prince?";
		next;
		cutin "ep19_lehar01.png",2;
		mes "[Lehar]";
		mes "A prince. That I'm not.";
		mes "My mother was the brother of the king and the youngest princess of the Gaebolg.";
		next;
		mes "[Lehar]";
		mes "My mother raised me in a monastery because she was concerned about the curse of the family...";
		mes "Her aunt was also in the monastery...";
		next;
		cutin "ep18_maram_01.png",0;
		mes "[Maram]";
		mes "Were you at the monastery to break the curse?";
		next;
		cutin "ep19_lehar04.png",2;
		mes "[Lehar]";
		mes "Yes. In the end, the curse was manifested by the son of the previous king, and my ended up in despair.";
		next;
		cutin "ep19_lehar01.png",2;
		mes "[Lehar]";
		mes "He was determined to break the curse of the family himself, then an opportunity arose and he went to Isgard, the origin of Jormungandr.";
		next;
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "What kind of place is Isgard?";
		next;
		cutin "ep19_lehar01.png",2;
		mes "[Lehar]";
		mes "It's a frozen land.";
		mes "It's a place full of snow, ice, and glaciers everywhere.";
		mes "Oh, but it's warm inside the castle.";
		next;
		cutin "ep19_lehar02.png",2;
		mes "[Lehar]";
		mes "For how they do it, I heard Leon is handling it with some kind of magic.";
		mes "Hahahaha...";
		mes "I don't remember.";
		next;
		cutin "ep19_leizi01.png",0;
		mes "[Lazy]";
		mes "Is there anything we should be careful about?";
		mes "I'm sure the Illusions are up to something.";
		next;
		cutin "ep19_lehar01.png",2;
		mes "[Lehar]";
		mes "The numbers of Rgans has dramatically increased recently, do you they have something to do with it?";
		mes "I think it would better to go see it for yourself...";
		next;
		cutin "ep172_beta.png",1;
		mes "[Ginger]";
		mes "The airship.";
		mes "Is about to land.";
		mes "Please prepare for a possible shock.";
		next;
		cutin "ep19_lehar01.png",2;
		mes "[Lehar]";
		mes "Oh! We're here!";
		mes "Shall we go to the Ice Castle?";
		completequest 18129;
		setquest 18124;
		close3;
	}
	cutin "ep19_lehar01.png",2;
	mes "[Lehar]";
	mes "Herlock's sleigh is good, but isn't the airship better?";
	mes "Shall we get off first?";
	mes "I'll take you to the Ice Castle!";
	close3;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(18129) == 1";
end;
}

air_if,31,58,1	script	Lazy#ep19lazy02	4_EP19_LAZY,{
	if(isbegin_quest(18123) == 1){
		cutin "ep19_leizi01.png",2;
		mes "[Lazy]";
		mes "Woah. The airship of the great sage is truly different from the inside out.";
		mes "I've been into Valkyrie Air's Airship a few times, but it wasn't to this extent.";
		close3;
	}
	if(isbegin_quest(18129) == 1){
		cutin "ep19_leizi01.png",2;
		mes "[Lazy]";
		mes "What do you think about my flying fashion?";
		mes "Is it cool?";
		next;
		mes "[Lazy]";
		mes "Is it cold there?";
		mes "You should wear your winter clothes tightly.";
		close3;
	}
	npctalk "Lazy: Is it landing already? My secret winter clothes will shine!","",bc_self;
	end;
}

air_if,29,62,5	script	Maram#ep19maram03	4_EP18_MARAM,{
	if(isbegin_quest(18123) == 1){
		npctalk "Maram: You can't even begin to compare it to a public airship. This airship is...!","",bc_self;
		end;
	}
	if(isbegin_quest(18129) == 1){
		cutin "ep18_maram_01.png",2;
		mes "[Maram]";
		mes "I'm going to discuss something later with Ginger, I will ask if I can use the airship from time to time.";
		mes "You will need someone to deliver if you need various resources, right?";
		close3;
	}
	npctalk "Maram: Are we finally here? I need to take a deep breath before getting off.","",bc_self;
	end;
}

air_if,29,59,6	script	Miriam#ep19miriam03	4_EP18_MIRIAM,{
	if(isbegin_quest(18123) == 1){
		npctalk "Miriam: It's the first time on such a long trip, I'm a little nervous.","",bc_self;
		end;
	}
	if(isbegin_quest(18129) == 1){
		npctalk "Miriam: I feel excited more then being nervous. I hope I can perform sufficiently.","",bc_self;
		end;
	}
	npctalk "Miriam: I'm a little nervous. Will it be cold?","",bc_self;
	end;
}

jor_tail,216,51,5	script	Navigator Ginger#ep19gg04	4_EP17_TABLET,5,5,{
	if(isbegin_quest(18124) == 1){
		cutin "ep172_beta.png",2;
		mes "[Ginger]";
		mes "Ginger will be waiting here.";
		mes "It's so that the airship can be available anytime!";
		close3;
	}
	cutin "ep172_beta.png",2;
	mes "[Ginger]";
	mes "Are you going to use the airship?";
	next;
	if(select("Yes, please.:No, I won't.") == 2){
		mes "[Ginger]";
		mes "Okay.";
		close3;
	}
	mes "[Ginger]";
	mes "It's cold here, so come on in.";
	mes "Departure is possible at any time.";
	close2;
	warp "air_if",53,71;
	end;
	
OnTouch:
	if(isbegin_quest(18124) == 1){
		cloaknpc("Lazy#ep19lazyjt",false,getcharid(0));
		cloaknpc("Maram#ep19maramjt",false,getcharid(0));
		cloaknpc("Miriam#ep19miriamjt",false,getcharid(0));
		cloaknpc("Lehar#ep19lehar04",false,getcharid(0));
	}
end;
}

jor_tail,221,53,3	script	Lehar#ep19lehar04	4_EP19_LEHAR,{
	if(isbegin_quest(18124) == 1){
		cutin "ep19_lehar01.png",0;
		mes "[Lehar]";
		mes "Ah, we luckily landed close2; to the Ice Castle.";
		mes "I usually go this way.";
		next;
		mes "[Lehar]";
		mes "They went out of the Ice Castle to meet us!";
		next;
		cutin "ep19_iwin11.png",2;
		mes "[Iwin Soldier]";
		mes "What is this";
		mes "Is it Lehar?";
		mes "A suspicious thing flew in, so I thought it was something hostile.";
		next;
		mes "[Iwin Soldier]";
		mes "Who are they?";
		mes "Are they your friends, Lehar?";
		mes "Who called for them, will they contribute?";
		next;
		cutin "ep19_lehar01.png",0;
		mes "[Lehar]";
		mes "Yes, yes. They're all in my party!";
		mes "Sice Leon also gave the permission, let's go to the Ice Castle.";
		next;
		mes "[Lehar]";
		mes "Let's go somewhere safe for now.";
		mes "Follow me!";
		completequest 18124;
		setquest 18125;
		close2;
		cutin "",255;
		navigateto("icas_in",141,216);
		end;
	}
	if(isbegin_quest(18125) == 1){
		cutin "ep19_lehar01.png",0;
		mes "[Lehar]";
		mes "Let's go somewhere safe for now.";
		mes "Follow me!";
		close2;
		cutin "",255;
		navigateto("icas_in",141,216);
	}
	end;

OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(18124) == 1";
end;
}

jor_tail,216,54,5	script	Lazy#ep19lazyjt	4_EP19_LAZY,{
	npctalk "Lazy: Wo~hoo~! This is it! Winter paradise!!!","",bc_self;
	end;
}

jor_tail,220,51,7	script	Maram#ep19maramjt	4_EP18_MARAM,{
	npctalk "Maram: Unbelievable.. I've never seen a place full of ice before..!!","",bc_self;;
	end;
}

jor_tail,223,50,7	script	Miriam#ep19miriamjt	4_EP18_MIRIAM,{
	npctalk "Miriam: When Lazy asked me to buy clothes.. I should have listened carefully...","",bc_self;
	end;
}

icas_in,141,216,3	script	Lehar#ep19lehar06	4_EP19_LEHAR,{
	if(isbegin_quest(18125) == 1){
		cutin "ep19_lehar01.png",2;
		mes "[Lehar]";
		mes "This is the place.";
		mes "This is an Ice Castle, but it's not cold inside, right?";
		cloaknpc("Lazy#ep19lazy03",false,getcharid(0));
		cloaknpc("Maram#ep19maram04",false,getcharid(0));
		cloaknpc("Miriam#ep19miriam04",false,getcharid(0));
		next;
		cloaknpc("Guardian Leon#ep19leon01",false,getcharid(0));
		cloaknpc("Guardian Aurelie#ep19arl01",false,getcharid(0));
		npctalk "Leon: Lehar. I guess these are the people who asked for the permission to get an invitation.","Guardian Leon#ep19leon01",bc_self;
		next;
		cutin "ep19_leon01.png",2;
		mes "[Leon Petit]";
		mes "I am Leon from the Petit family.";
		mes "I am playing the role of the guardian of this Ice Castle.";
		mes "Please feel free to call me Leon.";
		next;
		cutin "ep19_aurelie01.png",0;
		mes "[Aurelie Petit]";
		mes "You can call me Aurilie.";
		mes "You are a brilliant adventurer.";
		mes "It has been a long time since I saw an adventurer.";
		next;
		cutin "ep19_leon03.png",2;
		mes "[Leon Petit]";
		mes "According to Lehar Letter, that it has something to do with Isgard's recent problems.";
		mes "Can you tell me the details of the story?";
		next;
		select("Talk about the Illusion and Bagot.");
		cutin "ep19_aurelie03.png",0;
		mes "[Aurelie Petit]";
		mes "They took Fragment of Ymir's heart?";
		mes "Then, those suspicious dreams that I've been seeing ...";
		next;
		cutin "ep19_leon04.png",2;
		mes "[Leon Petit]";
		mes "It seems certain that part of the barrier has been breached.";
		mes "How much more has changed while you were asleep. ...";
		next;
		cutin "ep19_aurelie03.png",0;
		mes "[Aurelie Petit]";
		mes "There is a high possiblity that those people called the Illusions have colluded with the Rgans.";
		mes "We need to keep an eye on them.";
		next;
		cutin "ep19_leon01.png",2;
		mes "[Leon Petit]";
		mes "So far the Rgans haven't been able to cross the barrier yet, so that's ong good thing.";
		next;
		cutin "ep19_aurelie01.png",0;
		mes "[Aurelie Petit]";
		mes "I think it's just a matter of time.";
		mes "We need to prepare countermeasures.";
		mes "We also need to maintain the barrier.";
		next;
		cloaknpc("Vellgunde#ep19vell01",false,getcharid(0));
		cloaknpc("Voglinde#ep19vog01",false,getcharid(0));
		cutin "ep19_vellgunde01.png",2;
		mes "[Vellgunde]";
		mes "I was wondering who was making all those noises.";
		mes "Lehar, have you been into the city?";
		npctalk "Lazy: Who is it this time?","Lazy#ep19lazy03",bc_self;
		npctalk "Lehar: My aunt...","Lehar#ep19lehar06",bc_self;
		next;
		cutin "ep19_voglinde01.png",0;
		mes "[Voglinde]";
		mes "So. How was your errand?";
		mes "There are a lot of childrens I haven't seen before.";
		mes "Leon, Aurelie, have they introduce themselves?";
		next;
		cutin "ep19_leon02.png",2;
		mes "[Leon Petit]";
		mes "Oh, I made a mistake.";
		mes "I did not think of it before asking them questions.";
		mes "It has been a while since I have dealt with people...";
		next;
		cutin "ep19_aurelie04.png",0;
		mes "[Aurelie Petit]";
		mes "It is the same for me. I am sorry.";
		mes "The problems here are too serious too.";
		next;
		cutin "ep19_leizi01.png",2;
		mes "[Lazy]";
		mes "Oh, don't worry.";
		mes "We will tell you all the details from me and my friends here all you want later.";
		mes "I apologize for the late greetings. I'm Lazy from Schwarzwald.";
		next;
		cutin "ep18_miriam_02.png",0;
		mes "[Miriam]";
		mes "Thank you for your hospitality.";
		mes "I'm Miriam from Arunafeltz.";
		next;
		cutin "ep18_maram_01.png",2;
		mes "[Maram]";
		mes "I'm Maram, I'm also from Arunafeltz.";
		mes "I think I'll be indebted to you in the future. I look forward to working with you.";
		next;
		cutin "ep19_voglinde01.png",0;
		mes "[Voglinde]";
		mes "From Schwarzwald and Arunafeltz....";
		mes "It must have been a long story.";
		next;
		mes "[Voglinde]";
		mes "Childrens, can you fight?";
		mes "This place is barren, won't it be difficult if you're weak?";
		next;
		cutin "ep19_leizi01.png",2;
		mes "[Lazy]";
		mes "We are strong~";
		mes "Haha, your aunt is very interesting!";
		next;
		cutin "ep19_voglinde01.png",0;
		mes "[Voglinde]";
		mes "Oh look at you talking.";
		mes "I'm Lehar's aunt. So feel free to call me Aunt. ";
		next;
		mes "[Voglinde]";
		mes "I don't know if they can eat a spoonful of these green things properly.";
		mes "By the way, Lehar, have you finished the errand that I gave you?";
		next;
		cutin "ep19_lehar02.png",2;
		mes "[Lehar]";
		mes "Oh, yes. I did everything you asked. Should I tell you the details later?";
		next;
		cutin "ep19_voglinde01.png",0;
		mes "[Voglinde]";
		mes "Yes. Let's leave it for later.";
		mes "The reception for the childrens come first.";
		mes "I don't know if they have eaten properly yet.";
		next;
		mes "[Voglinde]";
		mes "Oh, right.";
		mes "I'm Voglinde from the Gaebolg family.";
		mes "I'm in charge of holding out the Rgans here.";
		mes "I'm glad to see that you're all talented children.";
		next;
		cutin "ep19_vellgunde01.png",2;
		mes "[Vellgunde]";
		mes "Oh, she's just being silly because she's meeting young ones again.";
		mes "I'm Vellgunde, I'm also from the Gaebolg family.";
		mes "I'm doing a lot of research here.";
		next;
		mes "[Vellgunde]";
		mes "I mainly research about the mana of Jormungandr and Rgans.";
		mes "I want to find a clue to the curse.";
		mes "Thanks to this, we also learned a little about the mana structure of the Rgans.";
		next;
		cutin "ep19_aurelie01.png",0;
		mes "[Aurelie Petit]";
		mes "It is a bit chaotic here since there are a lot of people.";
		mes "Lazy, was it?";
		mes "Can you share more information about the Illusions?";
		next;
		cutin "ep19_leizi01.png",2;
		mes "[Lazy]";
		mes "Sure. I prepared a thick document just in case you ask.";
		mes "Please call me anytime.";
		next;
		cutin "ep19_vellgunde01.png",1;
		mes "[Vellgunde]";
		mes "Hmm. Then, I'll go back to my laboratory.";
		mes "As I said, I'm doing my research here.";
		next;
		cutin "ep19_vellgunde03.png",1;
		mes "[Vellgunde]";
		mes "I'm using one side of the Iwin's barracks as a laboratory.";
		mes "Come and see me, if you want to talk more.";
		next;
		cutin "ep19_voglinde01.png",0;
		mes "[Voglinde]";
		mes "When you children are done talking, come over and have a meal.";
		close2;
		cutin "",255;
		cloaknpc("Vellgunde#ep19vell01",true,getcharid(0));
		cloaknpc("Voglinde#ep19vog01",true,getcharid(0));
		completequest 18125;
		setquest 18126;
		questinfo_refresh;
		end;
	}
	if(isbegin_quest(18126) == 1){
		npctalk "Lehar: Leon is waiting.","Lehar#ep19lehar06",bc_self;
		cloaknpc("Lazy#ep19lazy03",false,getcharid(0));
		cloaknpc("Maram#ep19maram04",false,getcharid(0));
		cloaknpc("Miriam#ep19miriam04",false,getcharid(0));
		cloaknpc("Guardian Leon#ep19leon01",false,getcharid(0));
		cloaknpc("Guardian Aurelie#ep19arl01",false,getcharid(0));
		end;
	}
	if(isbegin_quest(17648) == 2 && isbegin_quest(18132) == 0){
		cloaknpc("Miriam#ep19miriam04",false,getcharid(0));
		cutin "ep19_lehar01.png",2;
		mes "[Lehar]";
		mes "You're just in time.";
		mes "Leon said he needs some people, but there are only the two of us here.";
		next;
		mes "[Lehar]";
		mes "Where is Maram?";
		mes "Are you busy?";
		next;
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "Maram seems to be in charge of procurement and transporting supplies.";
		mes "I'm carrying stuffs or helping Vellgunde.";
		next;
		cutin "ep19_lehar01.png",2;
		mes "[Lehar]";
		mes "Is that so?";
		mes "I think two people is enough.";
		mes "Can the two of you go to Leon?";
		mes "I guess he really needs a hand.";
		mes "I have something else to do...";
		next;
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "If Leon called for me, then I'll go.";
		mes "Shall we go together?";
		setquest 18132;
		close2;
		cutin "",255;
		cloaknpc("Miriam#ep19miriam04",true,getcharid(0));
		end;
	}
	if(isbegin_quest(18132) == 1){
		cutin "ep19_lehar01.png",2;
		mes "[Lehar]";
		mes "Leon is probably in his room.";
		mes "The room on the left upstairs.";
		mes "Miriam already went there.";
		close3;
	}
	if(isbegin_quest(18128) == 1){
		cutin "ep19_lehar01.png",2;
		mes "[Lehar]";
		mes "I have some things that needs sorting out.";
		mes "Can you go and talk to my aunt first?";
		mes "I'll talk to the Iwins and catch up with you soon.";
		close3;
	}
	if(isbegin_quest(18127) == 1){
		cutin "ep19_lehar01.png",2;
		mes "[Lehar]";
		mes "The Iwin patrol unit is outside.";
		mes "They always patrol around here and monitor the Rgans.";
		mes "If you want to get to know the geography of the area, it'll be great to assist the Iwin patrol unit.";
		close3;
	}
	cutin "ep19_lehar01.png",2;
	mes "[Lehar]";
	mes "I stay here and help out my aunt with different things when she calls for me.";
	mes "Actually, my mother wanted to be with me, but her body was weak...";
	next;
	cutin "ep19_lehar02.png",2;
	mes "[Lehar]";
	mes "It's nice to have a spare time now that the adventurer is helping me with my work!";
	close3;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(18125) == 1";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17648) == 2 && isbegin_quest(18132) == 0";
end;
}

icas_in,138,220,4	script	Guardian Leon#ep19leon01	4_EP19_LEON,{
	if(isbegin_quest(18126) == 1){
		cutin "ep19_leon03.png",2;
		mes "[Leon Petit]";
		mes "Haha.. Is everything in order?";
		mes "Now, let me tell you a little about Isgard.";
		next;
		cutin "ep19_leon03.png",2;
		mes "[Leon Petit]";
		mes "Isgard is literally the lance of ice, but it's not really a land.";
		mes "By that, I mean geologically.";
		next;
		cutin "ep19_aurelie01.png",0;
		mes "[Aurelie Petit]";
		mes "Isgard is a land that formed from the frozen body of Jormungandr.";
		mes "And the Rgans, who was the main pillars of the Jormungandr cult is also sealed here.";
		next;
		select("What's the role of the barrier?");
		cutin "ep19_leon01.png",2;
		mes "[Leon Petit]";
		mes "The barrier is not to seal Jormungandr, It is a huge prison that prevents the Rgans from leaving this land.";
		next;
		cutin "ep18_maram_01.png",2;
		mes "[Maram]";
		mes "What's your role in here?";
		mes "Is it just Ice Castle...?";
		next;
		cutin "ep19_leizi01.png",0;
		mes "[Lazy]";
		mes "If this is a huge prison, then the residents here are the guards?";
		next;
		cutin "ep19_leon01.png",2;
		mes "[Leon Petit]";
		mes "You are not wrong.";
		mes "But please call them guardians if possible.";
		mes "Because calling the Iwins guardians makes them very proud.";
		next;
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "Iwin is the giant birds.. are you talking about them?";
		mes "It's my first time seeing them .. I'm sorry. It seems that my knowledge is lacking.";
		next;
		cutin "ep19_aurelie04.png",0;
		mes "[Aurelie Petit]";
		mes "Hahahaha. Iwins was a bird by nature.";
		mes "In fact, they were a group of migratory birds that is traveling back and forth across the sea and mountains.";
		next;
		cutin "ep19_aurelie02.png",0;
		mes "[Aurelie Petit]";
		mes "They were staying in Isgard to rest for a while, then got caught up in the seal.";
		mes "Ever since then, they have been helping us guard this place and monitor the barrier.";
		next;
		cutin "ep19_leon01.png",2;
		mes "[Leon Petit]";
		mes "That reminds me, would you like to meet the Iwins?";
		mes "They always patrol Isgard, so it would be helpful to hear their stories.";
		next;
		cutin "ep19_leon03.png",2;
		mes "[Leon Petit]";
		mes "Make yourself at home.";
		mes "Fortunately, the places that was made for Varmundt when he was still coming is still there...";
		next;
		select("Varmundt?");
		cutin "ep19_aurelie01.png",0;
		mes "[Aurelie Petit]";
		mes "It has been a while since he last came.";
		mes "Do you think he's doing well somewhere?";
		mes "Anyway, since you have come all the way here, I hope you can find what you are looking for.";
		next;
		cutin "ep19_leizi01.png",2;
		mes "[Lazy]";
		mes "Then, " + strcharinfo(0) + ", you should meet the Iwins.";
		mes "Me, Maram, and Miriam will discuss about what we're going to do here.";
		next;
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "That's a good idea.";
		mes "We will also explore the place and find countermeasures.";
		next;
		cutin "ep18_maram_01.png",2;
		mes "[Maram]";
		mes "Then, I'll take look into the situation of our supplies.";
		mes "If you need anything, I will make a route so that you can procure it without any inconvenience.";
		next;
		mes "[Maram]";
		mes "I'll have to discuss it with Ginger.";
		mes "Don't worry about us and do what you have to do first.";
		next;
		cutin "ep19_aurelie02.png",0;
		mes "[Aurelie Petit]";
		mes "Lehar, give it to them.";
		mes "And can you give the others a tour?";
		next;
		select("What is this?");
		cutin "ep19_lehar01.png",2;
		mes "[Lehar]";
		mes "It's a petal of a Snow Flower.";
		mes "You'll need it to live here.";
		mes "It's a crystallized mana that is like a flower, and it is used here for almost anything.";
		next;
		mes "[Lehar]";
		mes "I'll give you some, if you find anything useful, you can try using it.";
		mes "If you help the Iwins with their work also, they will reward you with this.";
		completequest 18126;
		setquest 18127;
		setquest 18128;
		getitem 1000608,40;
		addreputation("Ice Castle",50);
		next;
		mes "[Lehar]";
		mes "Then, I'll go and take care of the other things for a while.";
		close2;
		cutin "",255;
		cloaknpc("Lazy#ep19lazy03",true,getcharid(0));
		cloaknpc("Maram#ep19maram04",true,getcharid(0));
		cloaknpc("Miriam#ep19miriam04",true,getcharid(0));
		cloaknpc("Guardian Leon#ep19leon01",true,getcharid(0));
		cloaknpc("Guardian Aurelie#ep19arl01",true,getcharid(0));
		cloaknpc("Lehar#ep19lehar06",true,getcharid(0));
		end;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(18126) == 1";
end;
}

icas_in,141,220,3	duplicate(dummynpc2)	Guardian Aurelie#ep19arl01	4_EP19_AURELIE
icas_in,136,219,4	duplicate(dummynpc2)	Vellgunde#ep19vell01	4_EP19_VELLGUNDE
icas_in,144,218,3	duplicate(dummynpc2)	Voglinde#ep19vog01	4_EP19_VOGLINDE
icas_in,137,211,7	duplicate(dummynpc2)	Lazy#ep19lazy03	4_EP19_LAZY
icas_in,139,210,1	duplicate(dummynpc2)	Maram#ep19maram04	4_EP18_MARAM
icas_in,141,210,1	duplicate(dummynpc2)	Miriam#ep19miriam04	4_EP18_MIRIAM


icas_in,162,224,0	script	#aunterrand	HIDDEN_WARP_NPC,2,2,{
	end;
	
OnTouch:
	if(isbegin_quest(18128) == 1){
		mes "[Voglinde]";
		mes "Oh, it's you~!";
		mes "What's the matter?";
		mes "Where's Lehar?";
		npctalk "Voglinde : Have you seen Lehar?","Voglinde#ep19",bc_self;
		cloaknpc("Voglinde#ep19",false,getcharid(0));
		cutin "ep19_voglinde01.png",0;
		next;
		mes "[Voglinde]";
		mes "Did Lehar ask you to go here first because he has other stuffs to do?";
		mes "Anyway, he is a busy child.";
		npctalk "Voglinde : I see~","Voglinde#ep19",bc_self;
		next;
		mes "[Voglinde]";
		mes "I don't have time to wait for that boy since I don't know when he's coming, and I have something to ask of you. Do you have the time?";
		npctalk "Voglinde : Are you busy?","Voglinde#ep19",bc_self;
		next;
		if(select("It's okay.:I'm busy.") == 2){
			mes "[Voglinde]";
			mes "Are you?";
			mes "Then, there's nothing I can do. Ho~";
			mes "You are busy, so you better go to work.";
			cutin "ep19_voglinde02.png",0;
			close2;
			cloaknpc("Voglinde#ep19",true,getcharid(0));
			cloaknpc("Maram#ep19",false,getcharid(0));
			cutin "",255;
			end;
		}
		mes "[Voglinde]";
		mes "Really?";
		mes "That's great.";
		mes "You must be hungry after traveling so coming all the way here, eat this.";
		completequest 18128;
		setquest 11794;
		getitem Chocolate_Pie,1;
		cutin "ep19_voglinde04.png",0;
		cloaknpc("Maram#ep19",false,getcharid(0));
		next;
		mes "[Voglinde]";
		mes "It's nothing special, I just need to ask something about when you rode the airship.";
		mes "Have you rode with a cardinal with ^E5555Ea long hair and weird bag^000000?";
		npctalk "Voglinde : His bag is kind of like this.","Voglinde#ep19",bc_self;
		cutin "ep19_voglinde01.png",0;
		next;
		mes "[Voglinde]";
		mes "This much hair... No, is this too much?";
		mes "I've only sent her letters since I came here, I haven't seen her so I'm not sure.";
		cutin "ep19_voglinde02.png",0;
		next;
		mes "[Voglinde]";
		mes "The thing is... Lehar was supposed to bring her from ^E5555EProntera^000000.";
		mes "Didn't you see her?";
		mes "There was also no report that he is here...";
		cutin "ep19_voglinde01.png",0;
		next;
		mes "[Voglinde]";
		mes "He's not a child who's been around this kind of cold weather, so he probably not moving around that's why nobody else have seen him.";
		npctalk "Voglinde : He can't do that.","Voglinde#ep19",bc_self;
		cutin "ep19_voglinde04.png",0;
		next;
		select("I didn't see her.");
		mes "[Voglinde]";
		mes "Ugh...";
		mes "I'm in real trouble.";
		mes "I finally got the permission for her, and he just disappeared.";
		npctalk "Voglinde : Just thinking about hard it was to get that permission!!!","Voglinde#ep19",bc_self;
		cutin "ep19_voglinde03.png",0;
		next;
		mes "[Voglinde]";
		mes "Lehar also disappeared as soon we finished talking, I also didn't see where he went...";
		mes "This is not the time to be like this...";
		mes "Ugh...";
		npctalk "Voglinde : What should I do?","Voglinde#ep19",bc_self;
		next;
		mes "[Voglinde]";
		mes "Child.";
		mes "Can I ask a favor?";
		mes "This aunty is too busy to leave right now, so can you?";
		cutin "ep19_voglinde01.png",0;
		next;
		mes "[Voglinde]";
		mes "It's not that hard, didn't you ^E5555Erode the airship with the others^000000?";
		mes "I want you to ask them if they have seen a ^E5555Ecardinal with blue clothes, long hair, and a veil ^000000?";
		next;
		mes "[Voglinde]";
		mes "I can't tell you the details right now because of the situation, but it's very important.";
		mes "Then, please.";
		close2;
		cloaknpc("Voglinde#ep19",true,getcharid(0));
		cutin "",255;
		navigateto("icas_in",130,201);
		end;
	}
	if(isbegin_quest(11794) > 0 && isbegin_quest(11796) == 0){
		cloaknpc("Voglinde#ep19",false,getcharid(0));
		cutin "ep19_voglinde01.png",0;
		mes "[Voglinde]";
		mes "It's not that hard, didn't you ^E5555Erode the airship with the others^000000?";
		mes "I want you to ask them if they have seen a ^E5555Ecardinal with blue clothes, long hair, and a veil ^000000?";
		next;
		mes "[Voglinde]";
		mes "I can't tell you the details right now because of the situation, but it's very important.";
		mes "Then, please.";
		close2;
		cloaknpc("Voglinde#ep19",true,getcharid(0));
		cutin "",255;
		navigateto("icas_in",130,201);
		end;
	}
	if(isbegin_quest(11796) == 1){
		mes "[Voglinde]";
		mes "Sigh.";
		mes strcharinfo(0) + ", have you asked?";
		mes "Good job working in the cold.";
		npctalk "Voglinde : It's very cold outside, right?","Voglinde#ep19",bc_self;
		cloaknpc("Voglinde#ep19",false,getcharid(0));
		cutin "ep19_voglinde01.png",0;
		next;
		mes "[Voglinde]";
		mes "Did you find her?";
		next;
		select("Tell her honestly.");
		mes "[Voglinde]";
		mes "<FONT SIZE = 16><B>What!!!</FONT></B>";
		mes "<FONT SIZE = 16><B>I can't believe this.</FONT></B>";
		npctalk "Voglinde : Phew, I'm so frustrated!","Voglinde#ep19",bc_self;
		cutin "ep19_voglinde02.png",0;
		next;
		mes "[Lehar]";
		mes "No!";
		mes "Auntie, that's not it!";
		npctalk "Lehar : Auntie! Calm down!","Lehar#ep19_2",bc_self;
		cutin "ep19_lehar05.png",2;
		cloaknpc("Lehar#ep19_2",false,getcharid(0));
		next;
		mes "[Voglinde]";
		mes "<FONT SIZE = 16><B>What do you mean no!</FONT></B>";
		cutin "ep19_voglinde01.png",0;
		next;
		mes "[Voglinde]";
		mes "I told you to take care of it!";
		next;
		mes "[Voglinde]";
		mes "Sigh...";
		mes "...";
		mes "Child.";
		mes "Can I ask you one more favor?";
		next;
		mes "[Voglinde]";
		mes "I'm sorry, but could you return to ^E5555EMidgard^000000 and find her?";
		cutin "ep19_voglinde02.png",0;
		next;
		mes "[Voglinde]";
		mes "Because she's very important.";
		next;
		mes "[Voglinde]";
		mes "And you <B>shouldn't tell anyone about this.</B>";
		mes "Got it?";
		npctalk "Voglinde : It's strictly confidential. Be careful.","Voglinde#ep19",bc_self;
		cutin "ep19_voglinde04.png",0;
		next;
		mes "[Voglinde]";
		mes "I'll talk to ^E5555EHerlock^000000 about going to Midgard.";
		mes "Maybe he can take you all the way to ^E5555EAldebaran^000000.";
		mes "From there, you can go all the way to ^E5555EProntera^000000 by yourself, right?";
		cutin "ep19_voglinde03.png",0;
		next;
		mes "[Voglinde]";
		mes "Can you find a person called ^E5555ECopy Officer Ellis in Prontera^000000?";
		mes "She's probably near the ^E5555EProntera Cathedral^000000.";
		mes "She's a <B>saintess</B>, she'll know after you tell her that I sent you.";
		npctalk "Voglinde : I can't give you more details.","Voglinde#ep19",bc_self;
		next;
		mes "[Lehar]";
		mes "<FONT SIZE = 14>In that case, let me do it again!</FONT>";
		npctalk "Lehar : Auntie!!!","Lehar#ep19_2",bc_self;
		cutin "ep19_lehar07.png",2;
		next;
		mes "[Voglinde]";
		mes "You... come here for a second.";
		mes "Come.";
		cutin "ep19_voglinde02.png",0;
		next;
		mes "[Lehar]";
		mes "Auntie!!!";
		mes "I'm really sorry!";
		mes "You didn't even tell who it was!!";
		npctalk "Lehar : How would I know if I didn't even know her name or face!","Lehar#ep19_2",bc_self;
		cutin "ep19_lehar05.png",2;
		next;
		mes "[Voglinde]";
		mes "Why don't you know!!";
		mes "You've seen here before!!!";
		cutin "ep19_voglinde01.png",0;
		npctalk "Voglinde : Is that why you didn't even bother finding her!","Voglinde#ep19",bc_self;
		next;
		mes "[Lehar]";
		mes "That was many years ago!!!";
		cutin "ep19_lehar05.png",2;
		npctalk "Lehar : She was still a kid that last time I saw her!","Lehar#ep19_2",bc_self;
		next;
		mes "[Voglinde]";
		mes "Then, I look forward to you cooperation.";
		mes "^E5555EHerlock,^000000 should be near where you landed earlier, do you remember?";
		mes "Have you seen a  ^E5555Eweird-looking cabin^000000 there?";
		mes "He should be resting over there.";
		npctalk "Voglinde : Stop being loud and follow me to the prayer room.","Voglinde#ep19",bc_self;
		cutin "ep19_voglinde02.png",0;
		next;
		mes "[Lehar]";
		mes "Auntie!!";
		mes "Ouch!!";
		mes "Wait!!!!";
		mes strcharinfo(0) + "!";
		mes "Help me!";
		npctalk "Lehar : I'm going to die!!!!!!","Lehar#ep19_2",bc_self;
		cutin "ep19_lehar03.png",2;
		next;
		mes "[Voglinde]";
		mes "What do you mean die?";
		mes "Come over here!.";
		cutin "ep19_voglinde02.png",0;
		completequest 11796;
		setquest 11797;
		close2;
		cutin "",255;
		cloaknpc("Voglinde#ep19",true,getcharid(0));
		cloaknpc("Lehar#ep19_2",true,getcharid(0));
		navigateto("jor_tail",211,63);
		end;
	}
	if(isbegin_quest(11797) == 1){
		cloaknpc("Voglinde#ep19",false,getcharid(0));
		cloaknpc("Lehar#ep19_2",false,getcharid(0));
		mes "[Voglinde]";
		mes "Then, I look forward to you cooperation.";
		mes "^E5555EHerlock,^000000 should be near where you landed earlier, do you remember?";
		mes "Have you seen a  ^E5555Eweird-looking cabin^000000 there?";
		mes "He should be resting over there.";
		npctalk "Voglinde : Stop being loud and follow me to the prayer room.","Voglinde#ep19",bc_self;
		cutin "ep19_voglinde02.png",0;
		next;
		mes "[Lehar]";
		mes "Auntie!!";
		mes "Ouch!!";
		mes "Wait!!!!";
		mes strcharinfo(0) + "!";
		mes "Help me!";
		npctalk "Lehar : I'm going to die!!!!!!","Lehar#ep19_2",bc_self;
		cutin "ep19_lehar03.png",2;
		next;
		mes "[Voglinde]";
		mes "What do you mean die?";
		mes "Come over here!.";
		cutin "ep19_voglinde02.png",0;
		close2;
		cutin "",255;
		cloaknpc("Voglinde#ep19",true,getcharid(0));
		cloaknpc("Lehar#ep19_2",true,getcharid(0));
		navigateto("jor_tail",211,63);
		end;
	}
end;

OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(18128) == 1";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11796) == 1";
end;
}

icas_in,163,226,3	duplicate(dummynpc2)	Voglinde#ep19	4_EP19_VOGLINDE
icas_in,162,228,5	duplicate(dummynpc2)	Lehar#ep19_2	4_EP19_LEHAR


icas_in,130,201,5	script	Maram#ep19	4_EP18_MARAM,0,0,{
	if(isbegin_quest(11794) == 1){
		mes "[Maram]";
		mes "Rachel... Leaving Midgard, and coming all the way here feels so complicated and mysterious.";
		npctalk "Maram : It's so cold here, the Grey Wolf Village can't even be compared.","Maram#ep19",bc_self;
		cutin "ep18_maram_03.png",2;
		next;
		mes "[Maram]";
		mes "It's amazing because there's only ice here, and it's making me nervous since there are more things I have to be responsible for...";
		next;
		mes "[Maram]";
		mes "Oh... I'm not regreting the situation, it just feels strange, don't worry about it.";
		cutin "ep18_maram_02.png",2;
		next;
		mes "[Maram]";
		mes "You came here for something, right?";
		mes "What's going on?";
		cutin "ep18_maram_01.png",2;
		next;
		select("Ask about a cardinal.");
		mes "[Maram]";
		mes "I'm not sure?";
		mes "Was there someone like that?";
		next;
		mes "[Maram]";
		mes "If we rode the airship together, we wouldn't have missed that person...";
		mes "It's not like there were a lot of people.";
		npctalk "Maram : Were there any other people beside us?","Maram#ep19",bc_self;
		next;
		mes "[Maram]";
		mes "If she were in the cabin, we might not have been able to see her.";
		mes "But I don't think I saw her when we boarded and got off from the airship.";
		next;
		mes "[Maram]";
		mes "Honestly, since we have so little information and never met this person before, I don't think we'll reach an answer by just talking among ourselves, wouldn't it be better to ask ^E5555ELehar^000000?";
		cutin "ep18_maram_03.png",2;
		next;
		select("Ask Lehar's whereabouts.");
		mes "[Maram]";
		mes "Lehar, I think I saw going outside just a while ago...";
		next;
		mes "[Maram]";
		mes "Oh, he's coming.";
		mes "Lehar!!!";
		cutin "ep18_maram_02.png",2;
		completequest 11794;
		setquest 11795;
		close2;
		cutin "",255;
		cloaknpc("Lehar#ep19",false,getcharid(0));
		navigateto("icas_in",136,197);
		end;
	}
	if(isbegin_quest(11795) == 1){
		mes "[Maram]";
		mes "Lehar, I think I saw going outside just a while ago...";
		next;
		mes "[Maram]";
		mes "Oh, he's coming.";
		mes "Lehar!!!";
		cutin "ep18_maram_02.png",2;
		close2;
		cutin "",255;
		cloaknpc("Lehar#ep19",false,getcharid(0));
		navigateto("icas_in",136,197);
		end;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11794) == 1";
end;
}

icas_in,136,197,3	script	Lehar#ep19	4_EP19_LEHAR,0,0,{
	if(isbegin_quest(11795) == 1){
		mes "[Lehar]";
		mes "What is it?";
		mes "I'm busy right now, so if it's not that urgent, let's talk about it later.";
		cutin "ep19_lehar01.png",2;
		next;
		select("Ask about a cardinal.");
		mes "[Lehar]";
		mes "^E5555ECardinal with blue clothes, long hair, and a veil^000000, right?";
		mes "We even said our greetings at the airship.";
		npctalk "Lehar : Why are you asking, you're with her.","Lehar#ep19",bc_self;
		cutin "ep19_lehar02.png",2;
		next;
		mes "[Maram]";
		mes "We are with her?";
		cutin "ep18_maram_03.png",2;
		next;
		mes "[Lehar]";
		mes "Yes.";
		mes "Maram, don't you know her well?";
		cutin "ep19_lehar01.png",2;
		next;
		mes "[Miriam]";
		mes "Maram!";
		mes "What are you doing here!";
		mes "There's no time to relax!!";
		cutin "ep18_miriam_01.png",0;
		cloaknpc("Miriam#ep19",false,getcharid(0));
		next;
		mes "[Miriam]";
		mes "Hurry up and set our next schedule.";
		mes "I want to summarize what Voglinde said earlier.";
		next;
		mes "[Maram]";
		mes "Calm down, Miriam.";
		mes "We need to discuss something with you.";
		cutin "ep18_maram_01.png",2;
		next;
		mes "[Lehar]";
		mes "Just in time.";
		mes "^E5555ECardinal with blue clothes, long hair, and a veil!! ^000000.";
		cutin "ep19_lehar02.png",2;
		next;
		mes "[Maram]";
		mes "What...?";
		cutin "ep18_maram_01.png",2;
		next;
		mes "[Miriam]";
		mes "What?";
		mes "Eh... do I look like cardinal?";
		cutin "ep18_miriam_03.png",0;
		next;
		mes "[Lehar]";
		mes "...What?";
		cutin "ep19_lehar03.png",2;
		next;
		mes "- Sweat started pouring out from Lehar's forehead like a shower. -";
		next;
		mes "[Miriam]";
		mes "Lehar, what's with you suddenly?";
		mes "You don't look so good, is there a problem?";
		cutin "ep18_miriam_03.png",0;
		npctalk "Miriam : He's sweating!!!!!","Miriam#ep19",bc_self;
		next;
		mes "[Lehar]";
		mes "No... wait a minute, Miriam, the Prontera Cathedral... Through Ellie's introduction...";
		cutin "ep19_lehar03.png",2;
		npctalk "Lehar : Strictly confidential...","Lehar#ep19",bc_self;
		next;
		mes "[Miriam]";
		mes "I'm from Rachel, the same as Maram.";
		cutin "ep18_miriam_03.png",0;
		next;
		mes "[Lehar]";
		mes "Oh...";
		cutin "ep19_lehar04.png",2;
		next;
		mes "[Lehar]";
		mes "I think I made a mistake...";
		mes "I'm so screwed...";
		mes "...";
		cutin "ep19_lehar03.png",2;
		next;
		mes "[Maram]";
		mes "Well...";
		mes "I'm not really sure what's going on, but seeing Lehar like that, shouldn't we tell ^E5555EVoglinde^000000 about it first?";
		cutin "ep18_maram_03.png",2;
		next;
		select("That's a good idea.");
		mes "[Lehar]";
		mes "<FONT SIZE = 16><B>No!! Why is this happening!</FONT></B>";
		cutin "ep19_lehar03.png",2;
		next;
		mes "[Lehar]";
		mes "<FONT SIZE = 16><B>I'm going to die!</FONT></B>";
		mes strcharinfo(0) + "!!!!";
		cutin "ep19_lehar04.png",2;
		npctalk "Lehar : No!!!!!!!","Lehar#ep19",bc_self;
		completequest 11795;
		setquest 11796;
		questinfo_refresh;
		close2;
		cutin "",255;
		cloaknpc("Maram#ep19",true,getcharid(0));
		cloaknpc("Miriam#ep19",true,getcharid(0));
		cloaknpc("Lehar#ep19",true,getcharid(0));
		navigateto("icas_in",164,228);
		end;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11795) == 1";
end;
}

icas_in,132,201,3	duplicate(dummynpc2)	Miriam#ep19	4_EP18_MIRIAM

jor_tail,211,63,5	script	Herlock#ep19	4_EP19_HEALROCK,{
	if(isbegin_quest(11797) == 0){
		cutin "ep19_healrock01.png",2;
		mes "[Herlock]";
		mes "Euhahahaha!!!";
		mes "I'll tighten my deltoid muscles today!";
		close3;
	}
	if(isbegin_quest(11797) == 1){
		mes "[Herlock]";
		mes "Ooh~";
		mes "<FONT SIZE = 16><B>Hello.</FONT></B>";
		mes strcharinfo(0) + "!";
		mes "I've heard from Voglinde.";
		mes "Are you going to ^E5555EMidgard^000000?";
		cutin "ep19_healrock01.png",2;
		next;
		mes "[Herlock]";
		mes  strcharinfo(0) + ", are you the only going?";
		mes "We're too light... Maybe I should load some sandbags...";
		npctalk "Herlock : I'll just lose muscles, if I ran from here...","Herlock#ep19",bc_self;
		next;
		mes "[Herlock]";
		mes "Anyway, let's get going!";
		next;
		select("What's the airship?");	
		mes "[Herlock]";
		mes "The airship can only go to Varmundt Mansion.";
		mes "It's not something that we can decide...";
		next;
		mes "[Herlock]";
		mes "Don't worry, I can bring you to ^E5555EAldebaran^000000 using my sled.";
		mes "My broad shoulders will hold it firmly.";
		mes "Hahaha!!!";
		npctalk "Herlock : It's also a workout! It's two bird with one stone!","Herlock#ep19",bc_self;
		next;
		if(select("Depart.:Wait.") == 2){
			mes "[Herlock]";
			mes "Is that so?";
			mes "Tell me when you're ready.";
			close3;
		}
		mes "[Herlock]";
		mes "Then, let's go.";
		mes "Hold on tight!";
		completequest 11797;
		setquest 11798;
		close2;
		cutin "",255;
		warp "aldebaran",104,105;
		end;
	}
	if(isbegin_quest(11798) > 0 & isbegin_quest(11811) == 0){
		mes "[Herlock]";
		mes "Euhahahaha!!!!";
		mes "I'm just going to workout my lower body until Voglinde call me!!";
		cutin "ep19_healrock01.png",2;
		close3;
	}
	mes "[Herlock]";
	mes "Hello!!!!";
	mes "Are you going to Aldebaran again today?";
	cutin "ep19_healrock01.png",2;
	next;
	if(select("Depart.:Wait.") == 2){
		mes "[Herlock]";
		mes "Is that so?";
		mes "Tell me when you're ready.";
		close3;
	}
	mes "[Herlock]";
	mes "Then, let's go.";
	mes "Hold on tight!";
	close2;
	cutin "",255;
	warp "aldebaran",104,105;
	end;

OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11797) == 1";
end;
}

aldebaran,102,103,5	script	Herlock#ep19_a	4_EP19_HEALROCK,{
	//if(isbegin_quest(11620) < 2){
	//	mes "[Herlock]";
	//	mes "Hello.";
	//	mes "Adventurer, isn't the weather really nice?";
	//	cutin "ep19_healrock01.png",2;
	//	close3;
	//}
	if(isbegin_quest(11620) == 2 && isbegin_quest(11798) == 0){
		mes "[Herlock]";
		mes "Hello!!!!";
		mes "Let me see... Are you going to ^Varmundt Mansion^000000?";
		cutin "ep19_healrock01.png",2;
		next;
		mes "[Herlock]";
		mes "It's not under my jurisdiction... Do you want me to send you to a friend that can help?";
		next;
		if(select("Depart.:Wait.") == 2){
			mes "[Herlock]";
			mes "Is that so?";
			mes "Tell me when you're ready.";
			close3;
		}
		mes "[Herlock]";
		mes "Then, let's go.";
		mes "Hold on tight!";
		close2;
		cutin "",255;
		warp "pub_cat",89,102;
		end;
	}
	if(isbegin_quest(11798) == 1){
		mes "[Herlock]";
		mes "Ah! " + strcharinfo(0) + ".";
		mes "You've finally come to your senses!";
		mes "We're in Aldebaran.";
		cutin "ep19_healrock01.png",2;
		next;
		mes "[Herlock]";
		mes "I can't go all the way to Prontera because of various agreements, so I'll wait over here.";
		mes "Come back here to ^E5555EAldebaran^000000 when you're done.";
		npctalk "Herlock : Since you're here at Midgard, you should go shop for latest soy protein powder!","Herlock#ep19_a",bc_self;
		completequest 11798;
		setquest 11799;
		close2;
		cutin "",255;
		navigateto("prontera",254,309);
		end;
	}
	if(isbegin_quest(11799) == 1){
		mes "[Herlock]";
		mes "I can't go all the way to Prontera because of various agreements, so I'll wait over here.";
		mes "Come back here to ^E5555EAldebaran^000000 when you're done.";
		cutin "ep19_healrock01.png",2;
		npctalk "Herlock : Since you're here at Midgard, you should go shop for latest soy protein powder!","Herlock#ep19_a",bc_self;
		close2;
		cutin "",255;
		navigateto("prontera",254,309);
		end;
	}
	if(isbegin_quest(11808) == 1){
		mes "[Herlock]";
		mes "You're back.";
		mes "Then, right away... wait... I think we've got more people than what Voglinde said...";
		mes "Weren't you going with a Cardinal?";
		cutin "ep19_healrock01.png",2;
		next;
		mes "[Herlock]";
		mes "What's those in their hands?";
		mes "Is that luggage?";
		mes "One bag... two bags... No, wait a minute...";
		mes "It's not that I'm saying I can't carry it, but the burden on the sleigh...";
		cutin "ep19_healrock02.png",2;
		next;
		if(select("Depart.:Wait.") == 2){
			mes "[Herlock]";
			mes "Is that so?";
			mes "Tell me when you're ready.";
			close3;
		}
		mes "[Du]";
		mes "What???";
		cutin "ep18_dew_02.png",2;
		next;
		mes "[Herlock]";
		mes "Then, let's go...";
		mes "Urgh!!!!";
		cutin "ep19_healrock02.png",2;
		next;
		mes "[Herlock]";
		mes "Ugh...";
		mes "My... My arm...";
		cutin "ep19_healrock03.png",2;
		completequest 11808;
		setquest 11809;
		close2;
		cutin "",255;
		warp "jor_tail",212,60;
		end;
	}
	mes "[Herlock]";
	mes "Hello!!!!";
	mes "Are you going to ^E5555EIsgard^000000 today?";
	cutin "ep19_healrock01.png",2;
	next;
	if(select("Depart.:Wait.") == 2){
		mes "[Herlock]";
		mes "Is that so?";
		mes "Tell me when you're ready.";
		close3;
	}
	mes "[Herlock]";
	mes "Then, let's go.";
	mes "Hold on tight!";
	close2;
	cutin "",255;
	warp "jor_tail",212,60;
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11798) == 1";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11808) == 1";
end;
}

prontera,254,309,3	script	Copy Officer#ep19	4_F_ACOLYTE,{
	if(isbegin_quest(11799) == 1){
		mes "[Copy Officer Ellis]";
		mes "Oh!";
		mes "Hello.";
		mes "May I help you?";
		npctalk "Ellis : I don't think you're looking for a copy...","Copy Officer#ep19",bc_self;
		cutin "acact_01.bmp",2;
		next;
		select("I have an errand from Voglinde's errand.");
		mes "[Copy Officer Ellis]";
		mes "Are you Voglinde?";
		mes "...";
		cutin "acact_03.bmp",2;
		next;
		select("It's about the Saintess.");
		mes "[Copy Officer Ellis]";
		mes "If you know that far, then there's no need to hide it.";
		cutin "acact_01.bmp",2;
		next;
		mes "[Copy Officer Ellis]";
		mes "If it's her, Lehar was looking for her, so I sent her to the meeting place, Is there anything else you need?";
		cutin "acact_03.bmp",2;
		next;
		select("She disappeared.");
		mes "[Copy Officer Ellis]";
		mes "<FONT SIZE = 16><B>What?!</FONT></B>";
		mes "No...";
		mes "It's not as simple as she just disappeared!!!";
		cutin "acact_04.bmp",2;
		npctalk "Ellis : No!!!!!!!!!!!!!","Copy Officer#ep19",bc_self;
		next;
		mes "[Copy Officer Ellis]";
		mes "If you want to take her somewhere,you have to keep an eye on her all the time until you reach your destination...";
		mes "Sigh... Why did Lehar...";
		mes "...Nevermind...";
		next;
		mes "[Copy Officer Ellis]";
		mes "Let's go to her ^E5555Eroom^000000 first.";
		mes "If she's not wandering around somewhere, she'll be in her room.";
		mes "If she's not there... Then, it's a big problem.";
		completequest 11799;
		setquest 11800;
		close2;
		cutin "",255;
		navigateto("prt_cas",20,32);
		end;
	}
	if(isbegin_quest(11800) == 1){
		mes "[Copy Officer Ellis]";
		mes "If you want to take her somewhere,you have to keep an eye on her all the time until you reach your destination...";
		mes "Sigh... Why did Lehar...";
		mes "...Nevermind...";
		cutin "acact_04.bmp",2;
		next;
		mes "[Copy Officer Ellis]";
		mes "Let's go to her ^E5555Eroom^000000 first.";
		mes "If she's not wandering around somewhere, she'll be in her room.";
		mes "If she's not there... Then, it's a big problem.";
		close2;
		cutin "",255;
		navigateto("prt_cas",20,32);
		end;
	}
	mes "[Copy Officer Ellis]";
	mes "Oh!";
	mes "Welcome to the Prontera Cathedral.";
	mes "Lord Odin will always welcome you with open arms.";
	cutin "acact_02.bmp",2;
	close2;
	cutin "",255;
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11799) == 1";
end;
}

prt_cas,18,32,5	script	Saintess#ep19	4_EP19_FRIEDERIKE,{
	if(isbegin_quest(11801) == 1){
		mes "[Friederike]";
		mes "Hello.";
		mes strcharinfo(0) + ".";
		mes "I'm Friederike.";
		mes "Nice to meet you.";
		cutin "ep19_friederike03.png",2;
		cloaknpc("Traveling Bag#ep19_2",false,getcharid(0));
		next;
		mes "[Friederike]";
		mes "By the way, "+ strcharinfo(0) +", did Voglinde sent you?";
		mes "Is it cold in Isgard?";
		cutin "ep19_friederike02.png",2;
		cloaknpc("Traveling Bag#ep19_3",false,getcharid(0));
		next;
		select("It's cold.");
		mes "[Friederike]";
		mes "I don't like the cold.";
		mes "Is it true that houses there are made of ice too?";
		mes "I need to bring a cotton blanket!";
		cloaknpc("Traveling Bag#ep19_4",false,getcharid(0));
		next;
		select("There's also a castle made of ice.");
		mes "[Friederike]";
		mes "Wow!";
		mes "It must be pretty!";
		mes "Then, which color do you think will complement that place, this sky blue dress or this white dress?";
		cutin "ep19_friederike03.png",2;
		cloaknpc("Traveling Bag#ep19_5",false,getcharid(0));
		next;
		mes "[Friederike]";
		mes "Oh!";
		mes "I should also bring a sandal that fits my dress!";
		cutin "ep19_friederike01.png",2;
		cloaknpc("Traveling Bag#ep19_6",false,getcharid(0));
		next;
		mes "[Mark]";
		mes "How does your luggage keep increasing!";
		cloaknpc("Mark#ep19_cas",false,getcharid(0));
		cutin "ep18_mark_03.png",0;
		next;
		mes "[Mark]";
		mes "Oh!";
		mes "Hello, " + strcharinfo(0) + ".";
		cutin "ep18_mark_01.png",0;
		next;
		mes "[Copy Officer Ellis]";
		mes "Eh, isn't the bag getting bigger...";
		mes "Don't tell my that you're planning to take everything that's here?";
		cloaknpc("Ellis#ep19",false,getcharid(0));
		cutin "acact_04.bmp",2;
		next;
		mes "[Friederike]";
		mes "Can't I?";
		cutin "ep19_friederike02.png",2;
		next;
		mes "[Copy Officer Ellis]";
		mes "Of course not!";
		mes "Just bring what you really need!";
		cutin "acact_04.bmp",2;
		next;
		mes "[Friederike]";
		mes "I need all the stuffs.";
		cutin "ep19_friederike02.png",2;
		next;
		mes "[Copy Officer Ellis]";
		mes strcharinfo(0) + ", don't look at me.";
		mes "Even if she's like this, her skills are really good...";
		mes "skills...";
		mes "really...";
		mes "really....";
		cutin "acact_04.bmp",2;
		next;
		mes "[Copy Officer Ellis]";
		mes "I give up.";
		mes "If we don't open the bag and check it out, she's really going to bring all of this to Isgard.";
		next;
		mes "[Copy Officer Ellis]";
		mes "We should take a look at the bag and reduce it as much as possible, right?";
		cutin "acact_01.bmp",2;
		completequest 11801;
		setquest 11802;
		close3;
	}
	if(isbegin_quest(11802) > 0 && isbegin_quest(11808) == 0){
		cloaknpc("Traveling Bag#ep19_2",false,getcharid(0));
		cloaknpc("Traveling Bag#ep19_3",false,getcharid(0));
		cloaknpc("Traveling Bag#ep19_4",false,getcharid(0));
		cloaknpc("Traveling Bag#ep19_5",false,getcharid(0));
		cloaknpc("Traveling Bag#ep19_6",false,getcharid(0));
		cloaknpc("Mark#ep19_cas",false,getcharid(0));
		cloaknpc("Ellis#ep19",false,getcharid(0));
		mes "[Copy Officer Ellis]";
		mes "Eh, isn't the bag getting bigger...";
		mes "Don't tell my that you're planning to take everything that's here?";
		cutin "acact_04.bmp",2;
		next;
		mes "[Friederike]";
		mes "Can't I?";
		cutin "ep19_friederike02.png",2;
		next;
		mes "[Copy Officer Ellis]";
		mes "Of course not!";
		mes "Just bring what you really need!";
		cutin "acact_04.bmp",2;
		next;
		mes "[Friederike]";
		mes "I need all the stuffs.";
		cutin "ep19_friederike02.png",2;
		next;
		mes "[Copy Officer Ellis]";
		mes strcharinfo(0) + ", don't look at me.";
		mes "Even if she's like this, her skills are really good...";
		mes "skills...";
		mes "really...";
		mes "really....";
		cutin "acact_04.bmp",2;
		next;
		mes "[Copy Officer Ellis]";
		mes "I give up.";
		mes "If we don't open the bag and check it out, she's really going to bring all of this to Isgard.";
		next;
		mes "[Copy Officer Ellis]";
		mes "We should take a look at the bag and reduce it as much as possible, right?";
		cutin "acact_01.bmp",2;
		close3;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11801) == 1";
end;
}


prt_cas,20,32,3	script	Traveling Bag#ep19_1	4_EP19_SUITCASE,0,0,{
	if(isbegin_quest(11800) == 1){
		mes "[Copy Officer Ellis]";
		mes "Sigh!";
		mes "There is a bag, I guess she is here?";
		npctalk "Ellis : Phew... I've been through this for ten years.","Ellis#ep19",bc_self;
		cloaknpc("Ellis#ep19",false,getcharid(0));
		cutin "acact_03.bmp",2;
		next;
		cloaknpc("Saintess#ep19",false,getcharid(0));
		mes "[Saintess]";
		mes "You're here?";
		mes "That's nice!";
		mes "Which one is cuter, the coat or the pink one?";
		mes "I can't choose because they're both too cute.";
		cutin "ep19_friederike03.png",2;
		next;
		mes "[Copy Officer Ellis]";
		mes "Why are you still here?";
		mes "Didn't Lehar pick you up?";
		cutin "acact_04.bmp",2;
		next;
		mes "[Saintess]";
		mes "Lehar?";
		mes "Huh... When I went to the meeting place, this Lehar wasn't there but someone gave me a letter?";
		cutin "ep19_friederike01.png",2;
		next;
		mes "[Copy Officer Ellis]";
		mes "Le...tter...?";
		cutin "acact_04.bmp",2;
		next;
		mes "[Friederike]";
		mes "It said to come to Varmundt Mansion.";
		cutin "ep19_friederike02.png",2;
		next;
		mes "[Copy Officer Ellis]";
		mes "Then, why are you here?";
		mes "Didn't you go?";
		cutin "acact_04.bmp",2;
		next;
		mes "[Copy Officer Ellis]";
		mes "No, wouldn't the place be farther than the Varmundt Mansion?";
		mes "Did he tell you to come there alone?";
		next;
		mes "[Saintess]";
		mes "I did go, however...";
		mes "I was looking around at the Varmundt Mansion, then I got bored because no one was around...";
		mes "I've waited for a long time but no one came, so I went home.";
		cutin "ep19_friederike04.png",2;
		next;
		mes "[Copy Officer Ellis]";
		mes "Why didn't you call someone after getting there!.";
		cutin "acact_04.bmp",2;
		next;
		mes "[Copy Officer Ellis]";
		mes "...";
		next;
		mes "[Copy Officer Ellis]";
		mes "Anyway, that's no what I'm here for...";
		mes strcharinfo(0) + " is here to pick you up, so follow her.";
		mes "Voglinde is waiting for you.";
		cutin "acact_01.bmp",2;
		next;
		mes "[Saintess]";
		mes "Voglinde?";
		mes "I'm going to where Voglinde is?";
		cutin "ep19_friederike01.png",2;
		next;
		mes "[Saintess]";
		mes "Then, I should prepare my luggage for the trip!";
		next;
		mes "[Copy Officer Ellis]";
		mes "Luggage?????";
		mes "Then, organize it quickly.";
		mes "I'll get someone to carry it.";
		cutin "acact_04.bmp",2;
		next;
		mes "[Copy Officer Ellis]";
		mes strcharinfo(0) + ", can you watch her pack for a while?";
		mes "I'll be right back.";
		cutin "acact_01.bmp",2;
		completequest 11800;
		setquest 11801;
		close2;
		cutin "",255;
		cloaknpc("Ellis#ep19",true,getcharid(0));
		end;
	}
	if(isbegin_quest(11801) == 1){
		cloaknpc("Saintess#ep19",false,getcharid(0));
		mes "- Let's talk to the Saintess while waiting for Ellis. -";
		close;
	}
	if(isbegin_quest(11802) > 0 && isbegin_quest(11808) == 0){
		cloaknpc("Saintess#ep19",false,getcharid(0));
		cloaknpc("Traveling Bag#ep19_2",false,getcharid(0));
		cloaknpc("Traveling Bag#ep19_3",false,getcharid(0));
		cloaknpc("Traveling Bag#ep19_4",false,getcharid(0));
		cloaknpc("Traveling Bag#ep19_5",false,getcharid(0));
		cloaknpc("Traveling Bag#ep19_6",false,getcharid(0));
		cloaknpc("Mark#ep19_cas",false,getcharid(0));
		cloaknpc("Ellis#ep19",false,getcharid(0));
		mes "[Copy Officer Ellis]";
		mes "Eh, isn't the bag getting bigger...";
		mes "Don't tell my that you're planning to take everything that's here?";
		cutin "acact_04.bmp",2;
		next;
		mes "[Friederike]";
		mes "Can't I?";
		cutin "ep19_friederike02.png",2;
		next;
		mes "[Copy Officer Ellis]";
		mes "Of course not!";
		mes "Just bring what you really need!";
		cutin "acact_04.bmp",2;
		next;
		mes "[Friederike]";
		mes "I need all the stuffs.";
		cutin "ep19_friederike02.png",2;
		next;
		mes "[Copy Officer Ellis]";
		mes strcharinfo(0) + ", don't look at me.";
		mes "Even if she's like this, her skills are really good...";
		mes "skills...";
		mes "really...";
		mes "really....";
		cutin "acact_04.bmp",2;
		next;
		mes "[Copy Officer Ellis]";
		mes "I give up.";
		mes "If we don't open the bag and check it out, she's really going to bring all of this to Isgard.";
		next;
		mes "[Copy Officer Ellis]";
		mes "We should take a look at the bag and reduce it as much as possible, right?";
		cutin "acact_01.bmp",2;
		close3;
	}
	mes "- It's a traveling bag. -";
	close;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11800) == 1";
end;
}

prt_cas,15,31,5	script	Traveling Bag#ep19_2	4_EP19_SUITCASE,{
	if(isbegin_quest(11802) == 1){
		mes "- This bag is full of thick coats. -";
		next;
		mes "[Friederike]";
		mes "I'll need winter coats since Isgard is cold.";
		cutin "ep19_friederike01.png",2;
		next;
		mes "[Copy Officer Ellis]";
		mes "That's true.";
		mes "Then next!";
		cutin "acact_01.bmp",2;
		completequest 11802;
		setquest 11803;
		close3;
	}
	mes "- It's a traveling bag. -";
	close;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11802) == 1";
end;
}

prt_cas,16,27,3	script	Traveling Bag#ep19_3	4_EP19_SUITCASE,{
	if(isbegin_quest(11803) == 1){
		mes "- This bag is also full of thick coats. -";
		next;
		mes "[Friederike]";
		mes "It's cold in Isgard...";
		cutin "ep19_friederike02.png",2;
		next;
		mes "[Copy Officer Ellis]";
		mes "There are coats in the other bag too.";
		cutin "acact_01.bmp",2;
		next;
		mes "[Friederike]";
		mes "But...";
		cutin "ep19_friederike04.png",2;
		next;
		mes "[Copy Officer Ellis]";
		mes "You will not even go outside because it's cold.";
		mes "Just take the other bag!";
		mes "Then next!";
		cutin "acact_01.bmp",2;
		completequest 11803;
		setquest 11804;
		close3;
	}
	mes "- It's a traveling bag. -";
	close;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11803) == 1";
end;
}


prt_cas,26,27,3	script	Traveling Bag#ep19_4	4_EP19_SUITCASE,0,0,{
	if(isbegin_quest(11804) == 1){
		mes "- This is a teddy bear in this bag. -";
		next;
		mes "[Copy Officer Ellis]";
		mes "...";
		cutin "acact_01.bmp",2;
		next;
		mes "[Friederike]";
		mes "No~~!";
		mes "I can't sleep without Angelica!";
		cutin "ep19_friederike04.png",2;
		next;
		mes "[Copy Officer Ellis]";
		mes "Then, Only one doll.";
		cutin "acact_01.bmp",2;
		next;
		mes "[Friederike]";
		mes "Yes.";
		cutin "ep19_friederike03.png",2;
		next;
		mes "[Copy Officer Ellis]";
		mes "Then next!";
		cutin "acact_01.bmp",2;
		completequest 11804;
		setquest 11805;
		close3;
	}
	mes "- It's a traveling bag. -";
	close;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11804) == 1";
end;
}

prt_cas,23,36,3	script	Traveling Bag#ep19_5	4_EP19_SUITCASE,{
	if(isbegin_quest(11805) == 1){
		mes "- There are snacks in this bag. -";
		next;
		mes "[Copy Officer Ellis]";
		mes "It might be hard to get snacks in Isgard, so you can take this with you.";
		mes "Then next!";
		cutin "acact_01.bmp",2;
		completequest 11805;
		setquest 11806;
		close3;
	}
	mes "- It's a traveling bag. -";
	close;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11805) == 1";
end;
}

	
prt_cas,22,30,5	script	Traveling Bag#ep19_6	4_EP19_SUITCASE,{
	if(isbegin_quest(11806) == 1){
		mes "- There is a life-size Varmundt doll inside the bag. -";
		next;
		mes "[Copy Officer Ellis]";
		mes "Where did you even get this?";
		cutin "acact_04.bmp",2;
		next;
		mes "[Friederike]";
		mes "Alpha gave it to me when I went to the mansion.";
		cutin "ep19_friederike03.png",2;
		next;
		mes "[Copy Officer Ellis]";
		mes "Anyway, just get it back from me when you come back from Isgard.";
		cutin "acact_04.bmp",2;
		next;
		mes "[Friederike]";
		mes "Eh...";
		cutin "ep19_friederike04.png",2;
		next;
		mes "[Copy Officer Ellis]";
		mes "This should be enough!";
		cutin "acact_01.bmp",2;
		completequest 11806;
		setquest 11807;
		close3;
	}
	mes "- It's a traveling bag. -";
	close;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11806) == 1";
end;
}

prt_cas,21,31,3	script	Ellis#ep19	4_F_ACOLYTE,{
	if(isbegin_quest(11807) == 1){
		mes "[Copy Officer Ellis]";
		mes "Phew!";
		mes "It's still this big even though we cut down most of it.";
		mes "There won't be enough carriers...";
		cutin "acact_04.bmp",2;
		next;
		mes "[Du]";
		mes "Wow~!";
		mes "Mark is here!";
		mes "Oh!";
		mes "Why is also " + strcharinfo(0) + " here?";
		mes "Have you went to the new Stir-Fried Poring Restaunt in the shopping district?";
		cloaknpc("Du#ep19_cas",false,getcharid(0));
		cloaknpc("Maggi#ep19_cas",false,getcharid(0));
		cloaknpc("Alf#ep19_cas",false,getcharid(0));
		cutin "ep18_dew_01.png",2;
		next;
		mes "[Friederike]";
		mes "I want to eat there too!";
		cutin "ep19_friederike01.png",2;
		next;
		mes "[Copy Officer Ellis]";
		mes "Ug~~~grr!";
		mes "Du!";
		mes "Maggi!";
		mes "Alf!";
		cutin "acact_03.bmp",2;
		next;
		mes "[Copy Officer Ellis]";
		mes "Perfecting timing!";
		mes "I'll buy you your Stir-Fried Poring, so can you us move this luggages?";
		cutin "acact_02.bmp",2;
		next;
		mes "[Du]";
		mes "What?";
		mes "Really?";
		mes "Sister, are you going to buy it for us?";
		cutin "ep18_dew_05.png",2;
		next;
		mes "[Mark]";
		mes "Sister!";
		mes "That's...";
		mes "a bit...";
		cutin "ep18_mark_04.png",0;
		next;
		mes "[Copy Officer Ellis]";
		mes "Let me think...";
		cutin "acact_02.bmp",2;
		next;
		mes "[Copy Officer Ellis]";
		mes "Of course!";
		mes "I'll buy as many as you want.";
		next;
		mes "[Du]";
		mes "Nice!";
		mes "I'll be back like the wind, make sure you keep your wallet full!";
		cutin "ep18_dew_01.png",2;
		next;
		mes "[Copy Officer Ellis]";
		mes "Then " + strcharinfo(0) + ", I'll send you to Aldebaran.";
		mes "Please be careful on your way back.";
		mes "Also please take good care of Friederike.";
		cutin "acact_01.bmp",2;
		next;
		if(select("Move.:Quit.") == 2){
			mes "[Copy Officer Ellis]";
			mes "Tell me when you're ready.";
			close3;
		}
		mes "[Friederike]";
		mes "Then sister, I'll be back~";
		cutin "ep19_friederike02.png",2;
		next;
		mes "[Du]";
		mes "What? Aldebaran???";
		cutin "ep18_dew_02.png",2;
		next;
		mes "[Copy Officer Ellis]";
		mes "Enjoy your trip~";
		mes "Listen carefully to what Voglinde says.";
		cutin "acact_03.bmp",2;
		completequest 11807;
		setquest 11808;
		close2;
		cutin "",255;
		cloaknpc("Mark#ep19_cas",true,getcharid(0));
		cloaknpc("Ellis#ep19",true,getcharid(0));
		cloaknpc("Saintess#ep19",true,getcharid(0));
		cloaknpc("Traveling Bag#ep19_2",true,getcharid(0));
		cloaknpc("Traveling Bag#ep19_3",true,getcharid(0));
		cloaknpc("Traveling Bag#ep19_4",true,getcharid(0));
		cloaknpc("Traveling Bag#ep19_5",true,getcharid(0));
		cloaknpc("Traveling Bag#ep19_6",true,getcharid(0));
		cloaknpc("Du#ep19_cas",true,getcharid(0));
		cloaknpc("Maggi#ep19_cas",true,getcharid(0));
		cloaknpc("Alf#ep19_cas",true,getcharid(0));
		warp "aldebaran",104,105;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11807) == 1";
end;
}

prt_cas,22,33,3	duplicate(dummynpc2)	Mark#ep19_cas	4_EP18_MARK
prt_cas,23,32,3	duplicate(dummynpc2)	Du#ep19_cas	4_EP18_DEW
prt_cas,24,31,3	duplicate(dummynpc2)	Maggi#ep19_cas	4_4JOB_MAGGI
prt_cas,25,31,5	duplicate(dummynpc2)	Alf#ep19_cas	4_EP18_ALF

icecastle,23,123,0	script	#leharentrance	HIDDEN_WARP_NPC,3,3,{
	end;
	
OnTouch:
	if(isbegin_quest(11809) == 1){
		mes "[Lehar]";
		mes "Oh!";
		mes strcharinfo(0)+", you're finally here!";
		mes "The saintess...";
		cutin "ep19_lehar01.png",2;
		cloaknpc("Lehar#ep19_3",false,getcharid(0));
		cloaknpc("Friederike#ep19entrance",false,getcharid(0));
		cloaknpc("Du#ep19entrance",false,getcharid(0));
		cloaknpc("Mark#ep19entrance",false,getcharid(0));
		cloaknpc("Maggi#ep19entrance",false,getcharid(0));
		cloaknpc("Alf#ep19entrance",false,getcharid(0));
		next;
		mes "[Friederike]";
		mes "It's Friederike.";
		cutin "ep19_friederike01.png",2;
		next;
		mes "[Lehar]";
		mes "Hello.";
		mes "Friederike.";
		mes "and others...";
		mes "Thank you for coming to the Ice Castle.";
		cutin "ep19_lehar07.png",2;
		next;
		mes "[Lehar]";
		mes "I'm sorry for leaving you at Varmundt Mansion.";
		cutin "ep19_lehar04.png",2;
		next;
		mes "[Friederike]";
		mes "I'm fine.";
		mes "Did Voglinde scolded you a lot?";
		cutin "ep19_friederike02.png",2;
		next;
		mes "[Lehar]";
		mes "?.";
		mes "I can't remember...";
		cutin "ep19_lehar03.png",2;
		next;
		mes "[Du]";
		mes "What???";
		mes "Where are we!!!";
		mes "It's cold!!!";
		mes "My Stir-Fried Poring!!";
		cutin "ep18_dew_02.png",2;
		next;
		mes "[Mark]";
		mes "It's colder than I thought.";
		mes "We better move.";
		cutin "ep18_mark_01.png",0;
		next;
		mes "[Lehar]";
		mes "Oh!! Of course.";
		mes "Follow me, you shouldn't stay here.";
		mes "You'll catch a cold if you stay here for too long.";
		npctalk "Lehar : I don't like the prayer room...","Lehar#ep19_3",bc_self;
		cutin "ep19_lehar05.png",2;
		completequest 11809;
		setquest 11810;
		close2;
		cutin "",255;
		cloaknpc("Lehar#ep19_3",true,getcharid(0));
		cloaknpc("Friederike#ep19entrance",true,getcharid(0));
		cloaknpc("Du#ep19entrance",true,getcharid(0));
		cloaknpc("Mark#ep19entrance",true,getcharid(0));
		cloaknpc("Maggi#ep19entrance",true,getcharid(0));
		cloaknpc("Alf#ep19entrance",true,getcharid(0));
		navigateto("icecastle",59,213);
		end;
	}
end;

OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11809) == 1";
end;
}

icecastle,27,123,3	duplicate(dummynpc2)	Lehar#ep19_3	4_EP19_LEHAR
icecastle,25,121,5	duplicate(dummynpc2)	Friederike#ep19entrance	4_EP19_FRIEDERIKE
icecastle,20,126,5	duplicate(dummynpc2)	Du#ep19entrance	4_EP18_DEW
icecastle,21,127,5	duplicate(dummynpc2)	Mark#ep19entrance	4_EP18_MARK
icecastle,21,128,5	duplicate(dummynpc2)	Maggi#ep19entrance	4_4JOB_MAGGI
icecastle,22,129,5	duplicate(dummynpc2)	Alf#ep19entrance	4_EP18_ALF

icecastle,59,213,0	script	#in_house1	WARPNPC,1,1,{
	end;
	
OnTouch:
	if(isbegin_quest(11810) == 0)
		mes "- The door seems to be locked.";
	else
		warp "icas_in",33,112;
end;

OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11810) == 1";
end;
}

icas_in,33,112,0	script	#insiderestriction	HIDDEN_WARP_NPC,1,1,{
	end;
OnTouch:
	if(isbegin_quest(11811) == 2)
		cloaknpc("Syururu#ep19_room",false,getcharid(0));
	if(isbegin_quest(11815) > 0)
		cloaknpc("Tatarin#ep19_room",false,getcharid(0));
end;
}

icas_in,32,123,5	script	Friederike#ep19_room	4_EP19_FRIEDERIKE,{
	if(isbegin_quest(11810) == 1){
		mes "[Friederike]";
		mes "Is this my room?";
		mes "It's cute!";
		cutin "ep19_friederike03.png",2;
		cloaknpc("Lehar#ep19_room",false,getcharid(0));
		cloaknpc("Du#ep19_room",false,getcharid(0));
		cloaknpc("Mark#ep19_room",false,getcharid(0));
		cloaknpc("Maggi#ep19_room",false,getcharid(0));
		cloaknpc("Alf#ep19_room",false,getcharid(0));
		next;
		mes "[Voglinde]";
		mes "The childrens are here.";
		mes "You did a great job.";
		cutin "ep19_voglinde01.png",0;
		cloaknpc("Voglinde#ep19_room",false,getcharid(0));
		next;
		mes "[Friederike]";
		mes "Oh!!!";
		mes "Voglinde, I'm here~";
		cutin "ep19_friederike01.png",2;
		next;
		mes "[Mark]";
		mes "Ah... Hello!";
		mes "Your royal highness!!!";
		cutin "ep18_mark_01.png",0;
		next;
		mes "[Du]";
		mes "<FONT SIZE = 16><B>Wow... Your royal highness?!</FONT></B>";
		cutin "ep18_dew_02.png",2;
		next;
		mes "[Voglinde]";
		mes "Oh!";
		mes "You're Ellis' little sister.";
		mes "No need to be so formal.";
		mes "Just call me aunt Voglinde.";
		mes "Are they your friends?";
		cutin "ep19_voglinde01.png",0;
		next;
		mes "[Mark]";
		mes "Yes.";
		cutin "ep18_mark_01.png",0;
		next;
		mes "[Voglinde]";
		mes "Welcome!";
		mes "We're still short on hands.";
		mes "<FONT SIZE = 16><B>Hohoho!</FONT></B>";
		cutin "ep19_voglinde01.png",0;
		next;
		mes "[Voglinde]";
		mes "Now that I confirmed that Friederike is here, I'll go back.";
		mes "There's so much to do.";
		cutin "ep19_voglinde03.png",0;
		next;
		mes "[Friederike]";
		mes "Eh~";
		mes "Are you leaving already?";
		cutin "ep19_friederike04.png",2;
		npctalk "Friederike : I just met you!","Friederike#ep19_room",bc_self;
		next;
		mes "[Voglinde]";
		mes "I'll call you when I'm free.";
		mes "I also prepared a delicious tea and snacks.";
		cutin "ep19_voglinde03.png",0;
		next;
		mes "[Friederike]";
		mes "Really?";
		cutin "ep19_friederike03.png",2;
		next;
		mes "[Voglinde]";
		mes "Yes~~";
		mes "I asked them to prepare something special knowing that you're coming.";
		cutin "ep19_voglinde01.png",0;
		next;
		mes "[Voglinde]";
		mes "Oh no!";
		mes "I don't have much time!";
		mes "I really need to go.";
		mes "^E5555ELehar^000000, please stay here.";
		mes "I believe in you.";
		cutin "ep19_voglinde01.png",0;
		npctalk "Voglinde : You two, follow me.","Voglinde#ep19_room",bc_self;
		next;
		mes "[Lehar]";
		mes "Yes...? Yes!";
		cutin "ep19_lehar05.png",2;
		npctalk "Du : Yes?!","Du#ep19_room",bc_self;
		npctalk "Maggi : Yes!","Maggi#ep19_room",bc_self;
		cloaknpc("Voglinde#ep19_room",true,getcharid(0));
		cloaknpc("Du#ep19_room",true,getcharid(0));
		cloaknpc("Maggi#ep19_room",true,getcharid(0));
		completequest 11810;
		setquest 11811;
		questinfo_refresh;
		close3;
	}
	if(isbegin_quest(11811) == 1){
		mes "[Friederike]";
		mes "Lehar.";
		mes "What did Voglinde say?";
		cutin "ep19_friederike02.png",2;
		close3;
	}
	if(isbegin_quest(11820) == 1){
		mes "[Friederike]";
		mes "You're back?";
		mes "It's very cold, right?";
		mes "Come here everyone.";
		mes "It's very warm because of the feathers.";
		cutin "ep19_friederike02.png",2;
		next;
		mes "[Mark]";
		mes "I want to rest, but It's not the time for it.";
		cloaknpc("Mark#ep19_room",false,getcharid(0));
		cutin "ep18_mark_01.png",0;
		next;
		mes "[Mark]";
		mes "Put your coat on.";
		mes "We have a place to go.";
		next;
		mes "[Friederike]";
		mes "Where?";
		mes "Are we going outside?";
		mes "In the cold?";
		cutin "ep19_friederike02.png",2;
		next;
		mes "[Mark]";
		mes "We found a ^E5555Epit full of poison,^000000 it's probably there Tamarin fell into.";
		mes "There might be another victim if we don't purify it.";
		mes "Syururu's Mana Core has also broke.";
		cutin "ep18_mark_01.png",0;
		npctalk "Tatarin : I'm going to die!!!","Tatarin#ep19_room",bc_self;
		next;
		mes "[Friederike]";
		mes "He might just die if he fall in without the Mana Core.";
		mes "I don't want anyone to get hurt...";
		mes "Let's go.";
		cutin "ep19_friederike02.png",2;
		completequest 11820;
		setquest 11821;
		close2;
		cutin "",255;
		cloaknpc("Mark#ep19_room",true,getcharid(0));
		end;
	}
	if(isbegin_quest(11821) == 1){
		mes "[Friederike]";
		mes "He might just die if he fall in without the Mana Core.";
		mes "I don't want anyone to get hurt...";
		mes "Let's go.";
		cutin "ep19_friederike02.png",2;
		close3;
	}
	if(isbegin_quest(11829) == 1){
		mes "[Friederike]";
		mes "You're here?";
		mes "Are you hurt anywhere?";
		cutin "ep19_friederike02.png",2;
		cloaknpc("Mark#ep19_room",false,getcharid(0));
		cloaknpc("Alf#ep19_room",false,getcharid(0));
		next;
		mes "[Lehar]";
		mes "I'm so glad you're okay.";
		mes "I was surprised when I heard the news from Maggi.";
		cutin "ep19_lehar01.png",2;
		cloaknpc("Lehar#ep19_room",false,getcharid(0));
		next;
		mes "[Lehar]";
		mes "How's the inside?";
		mes "Depending on the scale of it's danger, we might not have to report it to aunt quickly...";
		cutin "ep19_lehar03.png",2;
		next;
		mes "[Voglinde]";
		mes "There's no need for that.";
		cutin "ep19_voglinde01.png",0;
		cloaknpc("Voglinde#ep19_room",false,getcharid(0));
		cloaknpc("Du#ep19_room",false,getcharid(0));
		cloaknpc("Maggi#ep19_room",false,getcharid(0));
		next;
		mes "[Lehar]";
		mes "<FONT SIZE = 16>Auntie!</FONT>";
		mes "How...";
		cutin "ep19_lehar05.png",2;
		next;
		mes "[Voglinde]";
		mes "Friederike here bragged about ^E5555Epurifying a decaying pit^000000.";
		mes "So I'm see to hear what's going on.";
		cutin "ep19_voglinde02.png",0;
		next;
		mes "[Voglinde]";
		mes "Looks like everybody came back safely.";
		mes "I'm glad~ that no one got hurt.";
		cutin "ep19_voglinde01.png",0;
		next;
		mes "[Voglinde]";
		mes "I'm proud, my childrens didn't fail my expectations.";
		mes "You're the best!";
		next;
		mes "[Voglinde]";
		mes "It's not a good news that such thing exist, but it's fortunate that we discover it before it turns into something big.";
		cutin "ep19_voglinde04.png",0;
		next;
		mes "[Syururu]";
		mes "<FONT SIZE = 14><B>Voglinde!</FONT></B>";
		mes "<FONT SIZE = 14><B>The pit was quite huge inside, but it didn't look like it was connected to another place.</FONT></B>";
		cutin "ep19_iwin03.png",2;
		next;
		mes "[Syururu]";
		mes "<FONT SIZE = 14><B>But the place is full of Rgan mutations and other monsters, It's still dangerous to leave it alone!</FONT></B>";
		next;
		mes "[Voglinde]";
		mes "Yeah?";
		mes "What is different about them?";
		cutin "ep19_voglinde01.png",0;
		next;
		mes "[Syururu]";
		mes "<FONT SIZE = 14><B>The Rgans were strangely twisted and have unnnatural movements.</FONT></B>";
		mes "<FONT SIZE = 14><B>The monsters had strange colors... thing that you won't see outside!</FONT></B>";
		cutin "ep19_iwin03.png",2;
		next;
		mes "[Voglinde]";
		mes "a strangely twisted Rgan...";
		mes "There is a possibility... that it might be a natural occurence, they might be those ^E5555EDiscarded Rgans^000000.";
		cutin "ep19_voglinde02.png",0;
		next;
		select("Discarded?");
		mes "[Voglinde]";
		mes "Rgans cruelly discriminate their people who don't meet their standards that they want.";
		mes "That's why they discard of them.";
		cutin "ep19_voglinde04.png",0;
		next;
		mes "[Syururu]";
		mes "<FONT SIZE = 14><B>I brought some meat samples here.</FONT></B>";
		cutin "ep19_iwin03.png",2;
		next;
		mes "[Voglinde]";
		mes "Oh... Is it really?";
		mes "Oh... It smells bad.";
		cutin "ep19_voglinde03.png",0;
		next;
		mes "[Voglinde]";
		mes "Can you purify this Friederike?";
		mes "My nose is going to be crooked.";
		cutin "ep19_voglinde02.png",0;
		next;
		mes "[Friederike]";
		mes "Yes.";
		mes "I already purified it.";
		cutin "ep19_friederike05.png",2;
		next;
		mes "[Syururu]";
		mes "Well.";
		mes "Since it's now purified, the smells is gone, and the meat is soft and savory.";
		cutin "ep19_iwin03.png",2;
		next;
		mes "[Du]";
		mes "Did you just eat it?!";
		cutin "ep18_dew_02.png",2;
		next;
		mes "[Voglinde]";
		mes "By out standards, it might be a little weird, but for the Iwins, the ^E5555ERgans^000000 are nothing more than a bug.";
		cutin "ep19_voglinde01.png",0;
		next;
		mes "[Syururu]";
		mes "I'm sure Lady Friederike will cure me if I ever get a stomachache.";
		mes "Whoo";
		cutin "ep19_iwin03.png",2;
		next;
		mes "[Syururu]";
		mes "Then, I'll go hunting again.";
		next;
		mes "[Lehar]";
		mes "Syururu, aren't you overdoing it?";
		mes "Why don't you take a break?";
		cutin "ep19_lehar05.png",2;
		next;
		mes "[Syururu]";
		mes "Tatarin is still injured, If you force yourself the ^E5555Efood you'll get will be low^000000.";
		cutin "ep19_iwin03.png",2;
		next;
		mes "[Lehar]";
		mes "I don't understand how you feel, but...";
		cutin "ep19_lehar04.png",2;
		next;
		mes "[Tatarin]";
		mes "I'll help you as soon as I recover, take a rest for now.";
		cutin "ep19_tamarin01.png",2;
		next;
		mes "[Syururu]";
		mes "Yes...";
		mes "Okay.";
		cutin "ep19_iwin03.png",2;
		next;
		mes "[Voglinde]";
		mes "Okay.";
		mes "I'll help the other chidlren to get food, so take a break for now.";
		mes "There's still a lot of work todo.";
		cutin "ep19_voglinde01.png",0;
		next;
		mes "[Voglinde]";
		mes "I'm still busy, so I'll...";
		mes "Listen to them, and have a good rest.";
		next;
		mes "[Lehar]";
		mes "You shouldn't worry about us too much, it would be good if you can take a rest too, Auntie...";
		mes "I'm worried.";
		cutin "ep19_lehar04.png",2;
		cloaknpc("Voglinde#ep19_room",true,getcharid(0));
		cloaknpc("Maggi#ep19_room",true,getcharid(0));
		cloaknpc("Du#ep19_room",true,getcharid(0));
		completequest 11829;
		getitem 1000608,10;
		addreputation("Ice Castle",10);
		close2;
		cutin "",255;
		cloaknpc("Alf#ep19_room",true,getcharid(0));
		cloaknpc("Mark#ep19_room",true,getcharid(0));
		cloaknpc("Lehar#ep19_room",true,getcharid(0));
		end;
	}
	if(checkquest(11813,HUNTING) == 2 && countitem(1000705) >= 10){
		mes "[Friederike]";
		mes strcharinfo(0) + "!!!";
		mes "You're back already.";
		mes "How was it?";
		mes "Did my blessing work?";
		cutin "ep19_friederike01.png",2;
		delitem 1000705,10;
		completequest 11813;
		setquest 11814;
		addreputation("Ice Castle",5);
		getitem 1000608,2 + (getreputation("Ice Castle") >= 3000 ? 1 : 0);
		next;
		mes "[Friederike]";
		mes "Wow!";
		mes "Look at all the Mana Core you brought, it must have worked somehow.";
		mes "It's not sticky either!";
		cutin "ep19_friederike03.png",2;
		next;
		mes "[Friederike]";
		mes "With an intact Mana Core like this, it would be easier to make something amazing!";
		mes "I have to tell Voglinde about todays results!";
		cutin "ep19_friederike05.png",2;
		next;
		mes "[Friederike]";
		mes "Thank you so much for your help.";
		mes strcharinfo(0) + ", Please get some rest.";
		mes "I will always pray for you.";
		cutin "ep19_friederike03.png",2;
		close3;
	}
	if(isbegin_quest(11811) == 2 && isbegin_quest(11812) == 0){
		mes "[Friederike]";
		mes "Come to think of it, I heard from Voglinde earlier.";
		cutin "ep19_friederike01.png",2;
		next;
		mes "[Friederike]";
		mes "Leon and Aurelie can purify the contaminated mana core of the Rgans, but they must be having a hard time because of the mana it consumes.";
		cutin "ep19_friederike02.png",2;
		next;
		mes "[Friederike]";
		mes "If we purify the mana core, ^E5555EVellgunde^000000 will...";
		mes "Well... that's how she explained it to me, I still don't know much about it.";
		cutin "ep19_friederike03.png",2;
		npctalk "Friederike : It's kinda of like this...","Friederike#ep19_room",bc_self;
		next;
		mes "[Friederike]";
		mes "Anyway, they extract it and transform it into various spell crystals.";
		cutin "ep19_friederike01.png",2;
		next;
		mes "[Friederike]";
		mes "It's being for several purposes.";
		mes "Making food, brightening the room......";
		npctalk "Friederike : For creating swim suits...","Friederike#ep19_room",bc_self;
		next;
		mes "[Friederike]";
		mes "Voglinde scolded me for saying that it was a waste to use the Mana Core that I purified for something like that.";
		cutin "ep19_friederike02.png",2;
		next;
		mes "[Friederike]";
		mes "Voglinde said that simple things like that can normally be enjoyed in places like Rune-Midgart which is rich in resources,";
		npctalk "Friederike : Trees don't grow here.","Friederike#ep19_room",bc_self;
		next;
		mes "[Friederike]";
		mes "In Isgard, brightening up the light in the rooms, purifying water, and every other little thing that is necessary to live.";
		next;
		mes "[Friederike]";
		mes "I'm the one who specialize in purification, So from now on, instead of Leon and Aurelie!";
		mes "I'll be the one purifying the Mana Cores!";
		cutin "ep19_friederike04.png",2;
		next;
		mes "[Friederike]";
		mes "Bring it to me, if " + strcharinfo(0) + " got a ^E5555EMana Core^000000 from the Rgan.";
		mes "It's my duty from now on.";
		next;
		mes "[Friederike]";
		mes "Oh!";
		mes "I also heard that there's a warehouse for a huge amount of Mana Cores that had been accumulated because no one was able to purify it,";
		npctalk "Friederike : That is also my duty that I will do in the future.","Friederike#ep19_room",bc_self;
		cutin "ep19_friederike01.png",2;
		next;
		mes "[Friederike]";
		mes "^E5555EMagic Cores^000000 are usually obtained from low-ranking Rgans.";
		mes "It's dirty and it's hard to process...";
		mes "The land around is polluted and it smells...";
		cutin "ep19_friederike02.png",2;
		next;
		mes "[Friederike]";
		mes "I'm figuring out how much blessing effect can the ^E5555ELow-Grade Magic Core^000000 from the Rgans can receive from my purification.";
		cutin "ep19_friederike01.png",2;
		next;
		mes "[Friederike]";
		mes "Figuring it out will make the work much easier.";
		mes strcharinfo(0) + ", if you're interested, would you like to help me?";
		cutin "ep19_friederike05.png",2;
		setquest 11812;
		completequest 11812;
		next;
	} else {
		mes "[Friederike]";
		mes "How is the Investigation on Isgard going?";
		mes "It's very cold outside, right?";
		mes "It would be nice get some rest in this room after working.";
		cutin "ep19_friederike02.png",2;
		next;
	}
	if(isbegin_quest(11812) == 2){
		switch(select("Help Friederike.:Purify Low-Grade Mana Core.:Small Talk.")){
			case 1:
				switch(checkquest(11814,PLAYTIME)){
					case -1:
						break;
						
					case 0:
					case 1:
						mes "[Friederike]";
						mes "With an intact Mana Core like this, it would be easier to make something amazing!";
						mes "I have to tell Voglinde about todays results!";
						cutin "ep19_friederike05.png",2;
						next;
						mes "[Friederike]";
						mes "Thank you so much for your help.";
						mes strcharinfo(0) + ", Please get some rest.";
						mes "I will always pray for you.";
						close3;
						
					case 2:
						erasequest 11814;
						break;
				}
				if(isbegin_quest(11813) == 0){
					mes "[Friederike]";
					mes "Then, ^E5555EPlease hunt 10^000000 ^E5555EPrimitive Rgan^000000 and ^E5555ELowest-Grade Rgan^000000, then bring me ^E5555E10 pieces^000000 or more ^E5555ERgans Mana Core^000000";
					cutin "ep19_friederike01.png",2;
					next;
					mes "[Friederike]";
					mes "If my predictions are correct, if you hunt down 10 Primitive and Lowest-Grade Rgans, It is expected to give more than 10 ^E5555EMana Core^000000, there might be a chance that you'll get a ^E5555EMana Core^000000 different from the lowest-grade.";
					next;
					mes "[Friederike]";
					mes "Don't overdo it and come back safely!";
					mes "Got it?";
					cutin "ep19_friederike04.png",2;
					setquest 11813;
				} else {
					mes "[Friederike]";
					mes "Then, ^E5555EPlease hunt 10^000000 ^E5555EPrimitive Rgan^000000 and ^E5555ELowest-Grade Rgan^000000, then bring me ^E5555E10 pieces^000000 or more ^E5555ERgans Mana Core^000000";
					cutin "ep19_friederike01.png",2;
					next;
					mes "[Friederike]";
					mes "If my predictions are correct, if you hunt down 10 Primitive and Lowest-Grade Rgans, It is expected to give more than 10 ^E5555EMana Core^000000, there might be a chance that you'll get a ^E5555EMana Core^000000 different from the lowest-grade.";
					next;
					mes "[Friederike]";
					mes "Don't overdo it and come back safely!";
					mes "Got it?";
					cutin "ep19_friederike04.png",2;
				}
				close3;
				
			case 2:
				mes "[Friederike]";
				mes "Great!";
				mes "So, shall I show off my skills?";
				next;
				if(countitem(1000707) < 10){
					mes "[Friederike]";
					mes "But...";
					mes "I need ^E5555E10 piece^000000 of ^E5555ERgans Low-Grade Mana Core^000000 to make something useful.";
					cutin "ep19_friederike02.png",2;
					next;
					mes "[Friederike]";
					mes "The cores mana is to weak...";
					close3;
				}
				mes "[Friederike]";
				mes "If you combine the mana core fragments of ^E5555ERgans Low-Grade Mana Core^000000!!!";
				mes "It will become a very useful ^E5555EPurified Mana Core^000000!";
				delitem 1000707,10;
				getitem 1000706,1;
				next;
				mes "[Friederike]";
				mes "^E5555EVellgunde^000000 will love it if you take it to her!";
				cutin "ep19_friederike03.png",2;
				close3;
				
			case 3:
				mes "[Friederike]";
				mes "Oh!";
				mes "What kind of interesting sotry are you going to tell me today?";
				cutin "ep19_friederike03.png",2;
				close3;
		}
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11810) == 1";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(11811) == 2 && isbegin_quest(11813) == 0 && checkquest(11814,PLAYTIME) == -1";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(11811) == 2 && isbegin_quest(11813) == 0 && checkquest(11814,PLAYTIME) == 2";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"checkquest(11813,HUNTING) == 2 && countitem(1000705) >= 10";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11820) == 1";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11829) == 1";
end;
}

icas_in,35,118,3	script	Lehar#ep19_room	4_EP19_LEHAR,{
	if(isbegin_quest(11811) == 1){
		mes "[Lehar]";
		mes "Then, Friederike can stay in this room.";
		mes "Call for me if you need anything...";
		mes "Is it okay for the other people who came with you stay here too?";
		mes "Friederike?";
		cutin "ep19_lehar01.png",2;
		next;
		mes "[Friederike]";
		mes "Yes";
		cutin "ep19_friederike01.png",2;
		next;
		mes "[Lehar]";
		mes "Later, I...";
		cutin "ep19_lehar01.png",2;
		next;
		mes "[???]";
		mes "<FONT SIZE = 16><B>Lehar!</FONT></B>";
		mes "<FONT SIZE = 16><B>Are you here?</FONT></B>";
		cutin "ep19_iwin03.png",2;
		cloaknpc("Syururu#ep19_room",false,getcharid(0));
		next;
		mes "[???]";
		mes "Oh!";
		mes "Are these the guests from Midgard?";
		mes "Sorry for coming in all of a sudden.";
		mes "It's something urgent...";
		next;
		mes "[Lehar]";
		mes "Syururu, what's going on?";
		cutin "ep19_lehar01.png",2;
		next;
		mes "[Syururu]";
		mes "Ah! It's not that...";
		mes "Do you have any ^E5555EPurified Mana Core^000000 left?";
		mes "My friend fell into the ^E5555Epit^000000 while we are hunting a while ago, but he is not waking up, he was wet and maybe there is a water that entered inside.";
		cutin "ep19_iwin03.png",2;
		next;
		mes "[Syururu]";
		mes "Unfortunately, my friends doesn't have any feathers, even though we collected feathers for him, my friends body remains cold.";
		mes "My friend will probably recover quickly with a ^E5555EPurified Mana Core^000000.";
		next;
		mes "[Syururu]";
		mes "I have a ^E5555ELow-Grade Mana Core^000000 from a Rgan...";
		mes "Its just ^E5555Ea useless stone if we don't purify it.^000000";
		next;
		mes "[Lehar]";
		mes "That's terrible!";
		mes "I hope your friend is safe...";
		mes "This is serious!";
		cutin "ep19_lehar05.png",2;
		next;
		mes "[Lehar]";
		mes "If it's a mana core, I think we can something somewhow, bring you friend here first.";
		cutin "ep19_lehar01.png",2;
		next;
		mes "[Syururu]";
		mes "Is it fine?";
		mes "^E5555ELeon and Aurelie^000000 didn't seem they have enough energy currently.";
		cutin "ep19_iwin03.png",2;
		next;
		mes "[Lehar]";
		mes "Don't worry about that!";
		cutin "ep19_lehar01.png",2;
		next;
		mes "[Syururu]";
		mes "<FONT SIZE = 16><B>Thank you</FONT></B>";
		mes "I'll be right back!";
		cutin "ep19_iwin03.png",2;
		cloaknpc("Syururu#ep19_room",true,getcharid(0));
		next;
		mes "[Lehar]";
		mes "Then, Friederike.";
		cutin "ep19_lehar01.png",2;
		next;
		mes "[Friederike]";
		mes "Huh?";
		mes "Why?";
		cutin "ep19_friederike01.png",2;
		next;
		mes "[Lehar]";
		mes "Oh!";
		mes "Please purify this mana core.";
		cutin "ep19_lehar03.png",2;
		next;
		mes "[Friederike]";
		mes "What's that dirty thing?";
		mes "Do I have to clean it?";
		mes "I don't want to touch it...";
		cutin "ep19_friederike02.png",2;
		next;
		mes "[Lehar]";
		mes "It's a request from my aunt.";
		cutin "ep19_lehar03.png",2;
		next;
		mes "[Friederike]";
		mes "Sigh...";
		mes "All I have to do is purify it, right?";
		cutin "ep19_friederike02.png",2;
		next;
		mes "[Lehar]";
		mes "Friederike won't get dirty in anyway.";
		cutin "ep19_lehar03.png",2;
		next;
		mes "[Friederike]";
		mes "But it will make me feel bad when I touch it...";
		cutin "ep19_friederike04.png",2;
		next;
		mes "[Friederike]";
		mes "Here I go.";
		mes "Is it like this?";
		cutin "ep19_friederike05.png",2;
		next;
		mes "[Lehar]";
		mes "Ha...";
		mes "As expected, it's so simple that it make our past effort futile.";
		cutin "ep19_lehar04.png",2;
		next;
		mes "[Lehar]";
		mes "Leon and Aurelie doesn't specialize in it but they can also purify the Mana Core little by little, they only do it when they really need to.";
		next;
		mes "[Lehar]";
		mes "It's not efficient for them when you consider the things you can do with the same amount of power.";
		next;
		mes "[Lehar]";
		mes "I can definitely understand why my aunt was looking for you desperately.";
		cutin "ep19_lehar03.png",2;
		next;
		mes "[Friederike]";
		mes "Really?";
		mes "Did Voglinde looked for me a lot?";
		cutin "ep19_friederike01.png",2;
		next;
		mes "[Lehar]";
		mes "If you continue to work hard to purify mana cores, my aunt will surely be more happy.";
		cutin "ep19_lehar01.png",2;
		next;
		mes "[Friederike]";
		mes "Hehe...";
		cutin "ep19_friederike03.png",2;
		completequest 11811;
		questinfo_refresh;
		getitem 1000608,30;
		addreputation("Ice Castle",50);
		close2;
		cutin "",255;
		cloaknpc("Syururu#ep19_room",false,getcharid(0));
		end;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11811) == 1";
end;
}

icas_in,27,116,5	script	Syururu#ep19_room	4_EP19_IWIN,{
	if(isbegin_quest(11811) < 2){
		cloaknpc(strnpcinfo(0),true,getcharid(0));
		end;
	}
	if(isbegin_quest(11815) == 0){
		mes "[Syururu]";
		mes "Lehar!!";
		mes "I brought him!";
		cutin "ep19_iwin03.png",2;
		cloaknpc("Tatarin#ep19_room",false,getcharid(0));
		next;
		mes "[Lehar]";
		mes "What... is this.";
		mes "This strange smell...";
		cutin "ep19_lehar03.png",2;
		cloaknpc("Lehar#ep19_room",false,getcharid(0));
		next;
		mes "[Syururu]";
		mes "I don't know the details either.";
		mes "We'll know when he wakes up.";
		cutin "ep19_iwin03.png",2;
		next;
		mes "[Lehar]";
		mes "Lady Friederike, please purify this person...";
		cutin "ep19_lehar03.png",2;
		next;
		mes "[Friederike]";
		mes "You keep making me do weird things...";
		mes "It smells...";
		cutin "ep19_friederike04.png",2;
		next;
		mes "[???]";
		mes "Ugh...";
		cutin "ep19_tamarin03.png",2;
		next;
		mes "[???]";
		mes "<FONT SIZE = 16><B>What!!!!</FONT></B>";
		cutin "ep19_tamarin02.png",2;
		next;
		mes "[Syururu]";
		mes "Tatarin, are you out of your mind?";
		mes "Are you okay!?";
		cutin "ep19_iwin03.png",2;
		next;
		mes "[Syururu]";
		mes "Thank you.";
		mes "Lehar!";
		mes "Lady... Friederike...?";
		next;
		mes "[Tatarin]";
		mes "Phew";
		mes "I managed to escape my death...";
		cutin "ep19_tamarin02.png",2;
		next;
		mes "[Du]";
		mes "Oh!!!";
		mes "Tamarin!!";
		mes "What are you doing here!";
		cutin "ep18_dew_02.png",2;
		cloaknpc("Du#ep19_room",false,getcharid(0));
		next;
		mes "[Tatarin]";
		mes "What!!";
		mes "Du!";
		mes "How did you get here!!!";
		cutin "ep19_tamarin02.png",2;
		next;
		mes "[Du]";
		mes "I don't know!!";
		cutin "ep18_dew_01.png",2;
		next;
		mes "[Mark]";
		mes "Seriously, what happened to you?";
		cloaknpc("Mark#ep19_room",false,getcharid(0));
		cutin "ep18_mark_01.png",0;
		next;
		mes "[Tatarin]";
		mes "Mark is also here?";
		cutin "ep19_tamarin02.png",2;
		next;
		mes "[Lehar]";
		mes "Do you know him?";
		mes "^E5555EYou can't come in without an invitation^000000, how did you get here?";
		cutin "ep19_lehar04.png",2;
		setquest 11815;
		questinfo_refresh;
		close2;
		cutin "",255;
		end;
	}
	if(isbegin_quest(11815) == 1){
		mes "[Syururu]";
		mes "Tatarin, are you out of your mind?";
		mes "Are you okay!?";
		cutin "ep19_iwin03.png",2;
		next;
		mes "[Syururu]";
		mes "Thank you.";
		mes "Lehar!";
		mes "Lady... Friederike...?";
		next;
		mes "[Tatarin]";
		mes "Phew";
		mes "I managed to escape my death...";
		cutin "ep19_tamarin02.png",2;
		next;
		mes "[Du]";
		mes "Oh!!!";
		mes "Tamarin!!";
		mes "What are you doing here!";
		cutin "ep18_dew_02.png",2;
		cloaknpc("Du#ep19_room",false,getcharid(0));
		next;
		mes "[Tatarin]";
		mes "What!!";
		mes "Du!";
		mes "How did you get here!!!";
		cutin "ep19_tamarin02.png",2;
		next;
		mes "[Du]";
		mes "I don't know!!";
		cutin "ep18_dew_01.png",2;
		next;
		mes "[Mark]";
		mes "Seriously, what happened to you?";
		cloaknpc("Mark#ep19_room",false,getcharid(0));
		cutin "ep18_mark_01.png",0;
		next;
		mes "[Tatarin]";
		mes "Mark is also here?";
		cutin "ep19_tamarin02.png",2;
		next;
		mes "[Lehar]";
		mes "Do you know him?";
		mes "^E5555EYou can't come in without an invitation^000000, how did you get here?";
		cutin "ep19_lehar04.png",2;
		close3;
	}
	if(isbegin_quest(11829) < 2){
		mes "[Syururu]";
		mes "When the feathers get wet, it'll feel heavy and you'll be prone to cold.";
		mes "It's always important to dry it indoors.";
		close3;
	}
	if(isbegin_quest(11849) == 1){
		mes "[Syururu]";
		mes "You're back.";
		mes "I'm really glad you're safe.";
		erasequest 11835;
		erasequest 11849;
		setquest 11850;
		addreputation("Ice Castle",5);
		getitem 1000608,2 + (getreputation("Ice Castle") >= 3000 ? 1 : 0);
		close3;
	}
	if(checkquest(11831,HUNTING) == 2 && countitem(1000708) >= 20){
		mes "[Syururu]";
		mes "You're back.";
		mes "I'm really glad you're safe.";
		erasequest 11831;
		setquest 11832;
		addreputation("Ice Castle",5);
		getitem 1000608,2 + (getreputation("Ice Castle") >= 3000 ? 1 : 0);
		close3;
	}
	if(checkquest(11833,HUNTING) == 2  && countitem(1000708) >= 20){
		mes "[Syururu]";
		mes "You're back.";
		mes "I'm really glad you're safe.";
		erasequest 11833;
		setquest 11834;
		addreputation("Ice Castle",5);
		getitem 1000608,2 + (getreputation("Ice Castle") >= 3000 ? 1 : 0);
		close3;
	}
	if(isbegin_quest(11830) == 0){
		mes "[Syururu]";
		mes "Phew...";
		mes "That pit was terrible.";
		cutin "ep19_iwin03.png",2;
		next;
		mes "[Syururu]";
		mes "Finding such a thing is such a headache on itself, fortunately there was no ^E5555ESuperior Rgan^000000.";
		next;
		mes "[Syururu]";
		mes "Those Rgans are without brains, they're just normal monsters.";
		next;
		mes "[Syururu]";
		mes "And the fact that we didn't find any Superior Rgans means that they don't care about that place.";
		next;
		mes "[Syururu]";
		mes "It won't get any worse if we block it well.";
		mes "If our efforts are not enough, we'll have to ask help from Lady Friederike.";
		next;
		mes "[Syururu]";
		mes "There will be other places to collect food if that happens, but there's another problem.";
		next;
		mes "[Syururu]";
		mes "We're low on manpower right now but I'll continue investigating the pit, so I hope you can help me when you have the time.";
		setquest 11830;
		completequest 11830;
	} else {
		mes "[Syururu]";
		mes "Hello.";
		mes "How are you feeling?";
		cutin "ep19_iwin03.png",2;
		next;
		mes "[Syururu]";
		mes "It's better to get a treatment if you feel any symptoms of poisoning.";
	}
	next;
	switch(select("Place of Disposal(1)-220lv:Place of Disposal(2)-240lv:Dig and Dig:Just here to chat.")){
		case 1:
			if(BaseLevel < 220){
				mes "[Syururu]";
				mes "It's too dangerous to go there now.";
				mes "We have a lot of other things to do";
				mes "why don't you try something else first?";
				cutin "ep19_iwin03.png",2;
				next;
				mes "^4D4DFFThis quest is for level 220 or above.^000000";
				close3;
			}
			switch(checkquest(11832,PLAYTIME)){
				case -1:
					break;
				case 0:
				case 1:
					mes "[Syururu]";
					mes "That's all for today!";
					mes "You've already helped a lot, so please get some rest.";
					cutin "ep19_iwin03.png",2;
					close3;
				case 2:
					erasequest 11832;
					break;
			}
			if(isbegin_quest(11831) == 0){
				mes "[Syururu]";
				mes strcharinfo(0) + ", I can't stop you from going to such a dangerous place again.";
				cutin "ep19_iwin03.png",2;
				next;
				mes "[Syururu]";
				mes "Then, please hunt down ^E5555E40 monsters^000000 on the ^E5555E1st floor of the Abandoned Pit^000000 and bring back ^E5555E20 pieces of Frozen Meat^000000.";
				cutin "ep19_iwin03.png",2;
				next;
				mes "[Syururu]";
				mes "If you think it's dangerous, please just come back.";
				mes "Nothing is more precious than your life.";
				setquest 11831;
				close3;
			}
			if(checkquest(11831,HUNTING) < 2){
				mes "[Syururu]";
				mes "Then, please hunt down ^E5555E40 monsters^000000 on the ^E5555E1st floor of the Abandoned Pit^000000 and bring back ^E5555E20 pieces of Frozen Meat^000000.";
				cutin "ep19_iwin03.png",2;
				next;
				mes "[Syururu]";
				mes "If you think it's dangerous, please just come back.";
				mes "Nothing is more precious than your life.";	
				close3;
			}
			end;
			
		case 2:
			if(BaseLevel < 240){
				mes "[Syururu]";
				mes "It's too dangerous to go there now.";
				mes "We have a lot of other things to do";
				mes "why don't you try something else first?";
				cutin "ep19_iwin03.png",2;
				next;
				mes "^4D4DFFThis quest is for level 240 or above.^000000";
				close3;
			}
			switch(checkquest(11834,PLAYTIME)){
				case -1:
					break;
				case 0:
				case 1:
					mes "[Syururu]";
					mes "That's all for today!";
					mes "You've already helped a lot, so please get some rest.";
					cutin "ep19_iwin03.png",2;
					close3;
				case 2:
					erasequest 11832;
					break;
			}
			if(isbegin_quest(11833) == 0){
				mes "[Syururu]";
				mes strcharinfo(0) + ", I can't stop you from going to such a dangerous place again.";
				cutin "ep19_iwin03.png",2;
				next;
				mes "[Syururu]";
				mes "Then, please hunt down ^E5555E40 monsters^000000 on the ^E5555E2nd floor of the Abandoned Pit^000000 and bring back ^E5555E20 pieces of Frozen Meat^000000.";
				cutin "ep19_iwin03.png",2;
				next;
				mes "[Syururu]";
				mes "If you think it's dangerous, please just come back.";
				mes "Nothing is more precious than your life.";
				setquest 11833;
				close3;
			}
			if(checkquest(11833,HUNTING) < 2){
				mes "[Syururu]";
				mes "Then, please hunt down ^E5555E40 monsters^000000 on the ^E5555E2nd floor of the Abandoned Pit^000000 and bring back ^E5555E20 pieces of Frozen Meat^000000.";
				cutin "ep19_iwin03.png",2;
				next;
				mes "[Syururu]";
				mes "If you think it's dangerous, please just come back.";
				mes "Nothing is more precious than your life.";	
				close3;
			}
			end;
			
		case 3:
			switch(checkquest(11850,PLAYTIME)){
				case -1:
					break;
				case 0:
				case 1:
					mes "[Syururu]";
					mes "That's all for today!";
					mes "You've already helped a lot, so please get some rest.";
					cutin "ep19_iwin03.png",2;
					close3;
				case 2:
					erasequest 11850;
					break;
			}
			mes "[Syururu]";
			mes "We haven't found any big problems in the abandoned pit, but we're always on the carefully looking out because we when or what might happen.";
			cutin "ep19_iwin03.png",2;
			next;
			mes "[Syururu]";
			mes "^E5555EAlf^000000 went to investigate again today, it's dangerous and takes a long time to do it alone, so please go to him and ask if he needs any help.";
			if(isbegin_quest(11835) == 0)
				setquest 11835;
			close2;
			cutin "",255;
			navigateto("job_ab01",115,14);
			end;
			
		case 4:
			mes "[Syururu]";
			mes "Hahahahha.";
			mes "We Iwins, the more feather we have the more attractive we are.";
			cutin "ep19_iwin03.png",2;
			close3;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11811) == 2 && isbegin_quest(11815) == 0";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11829) == 2 && isbegin_quest(11830) == 0";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"checkquest(11831,HUNTING) == 2  && countitem(1000708) >= 20";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"checkquest(11833,HUNTING) == 2  && countitem(1000708) >= 20";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11849) == 1";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(11829) == 2 && BaseLevel >= 220 && isbegin_quest(11831) == 0 && checkquest(11832,PLAYTIME) == -1";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(11829) == 2 && BaseLevel >= 220 && isbegin_quest(11831) == 0 && checkquest(11832,PLAYTIME) == 2";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(11829) == 2 && BaseLevel >= 240 && isbegin_quest(11833) == 0 && checkquest(11834,PLAYTIME) == -1";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(11829) == 2 && BaseLevel >= 240 && isbegin_quest(11833) == 0 && checkquest(11834,PLAYTIME) == 2";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11829) == 2 && isbegin_quest(11835) == 0 && checkquest(11850,PLAYTIME) == -1";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11829) == 2 && isbegin_quest(11835) == 0 && checkquest(11850,PLAYTIME) == 2";
end;
}

icas_in,27,122,5	script	Tatarin#ep19_room	4_EP19_TAMARIN,{
	if(isbegin_quest(11815) == 0){
		cloaknpc(strnpcinfo(0),true,getcharid(0));
		end;
	}
	if(isbegin_quest(11815) == 1){
		mes "[Tatarin]";
		mes "It's a long story...";
		cutin "ep19_tamarin01.png",2;
		next;
		mes "[Tatarin]";
		mes "I mean...";
		mes "It was when I was working on Varmundt Mansion.";
		mes "When I heard that the ungerground waterway broked again, I went ahead to repair, and the ^E5555EIllusion^000000 appeared and attacked me.";
		cutin "ep19_tamarin03.png",2;
		next;
		mes "[Tatarin]";
		mes "They brought me into something like an ^E5555Eairship^000000...";
		mes "When I came to my senses, I was in a ^E5555Erotten cave^000000.";
		next;
		mes "[Tatarin]";
		mes "There were other ^E5555Epeople that have been caught^000000 like me, I somehow managed to escape, but I don't know if the others are safe.";
		next;
		mes "[Tatarin]";
		mes "I managed to escape the cave and ran away, Syururu here found and saved me.";
		cutin "ep19_tamarin01.png",2;
		next;
		mes "[Syururu]";
		mes "Oh!";
		mes "Tatarin, weren't you an Ice Wind?";
		cutin "ep19_iwin03.png",2;
		next;
		mes "[Tatarin]";
		mes "I don't look like an Ice Wind no matter how much you look at me.";
		mes "No wonder he kept giving me feathers, saying that he felt sorry for me...";
		cutin "ep19_tamarin03.png",2;
		next;
		mes "[Syururu]";
		mes "Now I know!";
		mes "Then, let's move on from those trivial stuffs.";
		mes "What happened earlier, do you know how surprised I was when you fell on the floor?";
		cutin "ep19_iwin03.png",2;
		npctalk "Tatarin : What do you mean trivial...","Tatarin#ep19_room",bc_self;
		next;
		mes "[Syururu]";
		mes "I almost lost my mind, your body got colder and colder, and I thought you were going to die!";
		next;
		mes "[Tatarin]";
		mes "I was just hunting Limacina as usual...";
		mes "The floor suddenly cracked and I fell from it.";
		cutin "ep19_tamarin03.png",2;
		next;
		mes "[Tatarin]";
		mes "At first, I thought the Ice broke from the water, but the color was weird and something came out from it.";
		mes "And then... I don't remember.";
		cutin "ep19_tamarin01.png",2;
		next;
		mes "[Lehar]";
		mes "Well...";
		mes "Isn't that dangerous?";
		cutin "ep19_lehar03.png",2;
		cloaknpc("Lehar#ep19_room",false,getcharid(0));
		next;
		mes "[Tatarin]";
		mes "I think so.";
		mes "Come to think of it, the trap that I looked at before falling into the pit ^E5555Ewas also broken^000000.";
		mes "It was because of that, the trapped Limacina escaped and attacked me.";
		cutin "ep19_tamarin02.png",2;
		next;
		mes "[Syururu]";
		mes "It's common for traps to break.";
		cutin "ep19_iwin03.png",2;
		next;
		mes "[Tatarin]";
		mes "That's not it... there were traces of someone intentionally breaking it.";
		cutin "ep19_tamarin03.png",2;
		next;
		mes "[Syururu]";
		mes "I can't help but doubt that pit...";
		cutin "ep19_iwin03.png",2;
		next;
		mes "[Tatarin]";
		mes "I think it would be a good idea to take a look into it.";
		mes "It's not good if someone gets hurt.";
		cutin "ep19_tamarin03.png",2;
		next;
		mes "[Syururu]";
		mes "Don't overdo it!";
		mes "You should rest.";
		mes "I will go and check it out.";
		cutin "ep19_iwin03.png",2;
		next;
		mes "[Mark]";
		mes "We'll help you too.";
		mes "It will be a problem if you get into an accident while investigating by yourself.";
		cutin "ep18_mark_01.png",0;
		next;
		mes "[Syururu]";
		mes "Thank you.";
		mes "Then, please investigate the traps around here.";
		mes "I'll look around the place where I found Tatarin first.";
		cutin "ep19_iwin03.png",2;
		next;
		select("It's dangerous.");
		mes "[Syururu]";
		mes "Don't worry.";
		mes "I have some immunity to some extent because I'm wearing a ^E5555Edivine equipment reinforced with purified Mana Core^000000.";
		next;
		mes "[Syururu]";
		mes "Then if you find something while ^E5555Einvestigating the traps^000000, please come to the northwest of the Frozen Scale Plains.";
		completequest 11815;
		setquest 11816;
		close2;
		cutin "",255;
		cloaknpc("Lehar#ep19_room",true,getcharid(0));
		cloaknpc("Du#ep19_room",true,getcharid(0));
		cloaknpc("Mark#ep19_room",true,getcharid(0));
		navigateto("jor_back1",300,272);
		end;
	}
	if(isbegin_quest(11816) == 1){
		mes "[Tatarin]";
		mes "Come to think of it, the trap that I looked at before falling into the pit ^E5555Ewas also broken^000000.";
		cutin "ep19_tamarin02.png",2;
		next;
		mes "[Tatarin]";
		mes "There were traces of someone intentionally breaking it.";
		cutin "ep19_tamarin03.png",2;
		next;
		mes "[Tatarin]";
		mes "I think it would be a good idea to take a look into it.";
		mes "It's not good if someone gets hurt.";
		cutin "ep19_tamarin03.png",2;
		close2;
		cutin "",255;
		navigateto("jor_back1",300,272);
		end;
	}
	if(isbegin_quest(11829) < 2){
		mes "[Tatarin]";
		mes "Ah... I thought I was going to die.";
		cutin "ep19_tamarin03.png",2;
		close3;
	}
	if(checkquest(11852,HUNTING) == 2 && countitem(1000708) >= 10){
		mes "[Tatarin]";
		mes "Uh...";
		mes strcharinfo(0) + ", you're here.";
		mes "I dozed off a bit.";
		cutin "ep19_tamarin02.png",2;
		delitem 1000708,10;
		erasequest 11852;
		setquest 11853;
		addreputation("Ice Castle",5);
		getitem 1000608,2 + (getreputation("Ice Castle") >= 3000 ? 1 : 0);
		next;
		mes "[Tatarin]";
		mes "You did a great job.";
		mes "I'll recover soon and join you.";
		cutin "ep19_tamarin04.png",2;
		close3;
	}
	if(isbegin_quest(11858) == 1){
		mes "[Tatarin]";
		mes "Uh...";
		mes strcharinfo(0) + ", you're here.";
		mes "I dozed off a bit.";
		cutin "ep19_tamarin02.png",2;
		erasequest 11854;
		erasequest 11855;
		erasequest 11856;
		erasequest 11857;
		erasequest 11858;
		setquest 11859;
		addreputation("Ice Castle",5);
		getitem 1000608,2 + (getreputation("Ice Castle") >= 3000 ? 1 : 0);
		next;
		mes "[Tatarin]";
		mes "You did a great job.";
		mes "I'll recover soon and join you.";
		cutin "ep19_tamarin04.png",2;
		close3;
	}
	if(isbegin_quest(11851) == 0){
		mes "[Tatarin]";
		mes "Ugh... my head...";
		mes "My heart is pounding...";
		mes "My feet is also tingling...";
		cutin "ep19_tamarin03.png",2;
		next;
		mes "[Tatarin]";
		mes "My body is not recovering as I thought it would.";
		cutin "ep19_tamarin01.png",2;
		next;
		mes "[Tatarin]";
		mes "If I don't go hunting quickly, the stock on the refrigerator will run out quickly.";
		cutin "ep19_tamarin02.png",2;
		next;
		mes "[Tatarin]";
		mes "Lady Friederike told me that I was lucky that I didn't die, but...";
		cutin "ep19_tamarin04.png",2;
		next;
		mes "[Tatarin]";
		mes "Now, Syururu also can't participate in hunt because of the investigation of the Pit, the other hunters are busy working too.";
		next;
		mes "[Tatarin]";
		mes "Because of the environment, the hunting tools and traps turns to crisp quickly.";
		cutin "ep19_tamarin01.png",2;
		next;
		mes "[Tatarin]";
		mes strcharinfo(0) + ", if you catch monsters, don't just leave it and bring it back.";
		next;
		mes "[Tatarin]";
		mes "If you put it in the refrigerator, you can eat it fresh even after a long time.";
		cutin "ep19_tamarin04.png",2;
		setquest 11851;
		completequest 11851;
	} else {
		mes "[Tatarin]";
		mes "Ugh... my head...";
		mes "My heart is pounding...";
		mes "My feet is also tingling...";
		cutin "ep19_tamarin03.png",2;
		next;
		mes "[Tatarin]";
		mes "My body is not recovering as I thought it would.";
		cutin "ep19_tamarin01.png",2;
	}
	next;
	switch(select("Do you need the Refrigerator filled?:Crisped tools.:He's worried")){
		case 1:
			switch(checkquest(11853,PLAYTIME)){
				case -1:
					break;
				case 0:
				case 1:
					mes "[Tatarin]";
					mes "That's enough for today.";
					mes "Don't overdo it and take a good rest.";
					mes "We will need you for tomorrow's meat!";
					cutin "ep19_tamarin04.png",2;
					close3;
				case 2:
					erasequest 11853;
					break;
			}
			if(isbegin_quest(11852) == 0){
				mes "[Tatarin]";
				mes "Of course!";
				mes "If you just leave the meat outside in this weather, it will freeze and crumble.";
				cutin "ep19_tamarin04.png",2;
				next;
				mes "[Tatarin]";
				mes "I'll ask the other Iwins to prepare the fridge, so " + strcharinfo(0) + " just need to get the meat.";
				next;
				mes "[Tatarin]";
				mes "Then, hunt about ^E5555E20 monsters^000000 on the ^E5555EFrozen Scale Plain^000000 and collect atleast ^E5555E10 Frozen Meat^000000.";
				next;
				mes "[Tatarin]";
				mes "I think we need to fill up the refrigerator so I can just stretch and rest.";
				cutin "ep19_tamarin03.png",2;
				setquest 11852;
				close2;
				cutin "",255;
				navigateto("jor_back2",211,26);
			} else {
				mes "[Tatarin]";
				mes "I'll ask the other Iwins to prepare the fridge, so "+ strcharinfo(0) +" just need to get the meat.";
				cutin "ep19_tamarin04.png",2;
				next;
				mes "[Tatarin]";
				mes "Then, hunt about ^E5555E20 monsters^000000 on the ^E5555EFrozen Scale Plain^000000 and collect atleast ^E5555E10 Frozen Meat^000000.";
				next;
				mes "[Tatarin]";
				mes "I think we need to fill up the refrigerator so I can just stretch and rest.";
				cutin "ep19_tamarin03.png",2;
				close2;
				cutin "",255;
				navigateto("jor_back2",211,26);
			}
			end;
			
		case 2:
			switch(checkquest(11859,PLAYTIME)){
				case -1:
					break;
				case 0:
				case 1:
					mes "[Tatarin]";
					mes "That's enough for today.";
					mes "Don't overdo it and take a good rest.";
					mes "We will need you for tomorrow's meat!";
					cutin "ep19_tamarin04.png",2;
					close3;
				case 2:
					erasequest 11853;
					break;
			}
			if(isbegin_quest(11854) == 0){
				mes "[Tatarin]";
				mes "It's hard to believe, but it's the truth.";
				mes "That's why I have to keep maintaining it.";
				cutin "ep19_tamarin02.png",2;
				next;
				mes "[Tatarin]";
				mes "In the past, I tried to fix a trap that had been unattended for a few days, and it turned into a powder and got blew away by the wind.";
				next;
				mes "[Tatarin]";
				mes "Before that happens, we have to check the ^E5555Etraps on the Frozen Scale Plain^000000.";
				next;
				mes "[Tatarin]";
				mes "It's harder to find new materials to make one when they break.";
				cutin "ep19_tamarin03.png",2;
				setquest 11854;
				close2;
				cutin "",255;
				navigateto("jor_back2",367,36);
			} else {
				mes "[Tatarin]";
				mes "In the past, I tried to fix a trap that had been unattended for a few days, and it turned into a powder and got blew away by the wind.";
				next;
				mes "[Tatarin]";
				mes "Before that happens, we have to check the ^E5555Etraps on the Frozen Scale Plain^000000.";
				next;
				mes "[Tatarin]";
				mes "It's harder to find new materials to make one when they break.";
				cutin "ep19_tamarin03.png",2;
				close2;
				cutin "",255;
				navigateto("jor_back2",367,36);
			}
			end;
		
		case 3:
			mes "[Tatarin]";
			mes "It's okay.";
			mes "I'll get better soon.";
			cutin "ep19_tamarin04.png",2;
			close3;
	}
	end;

OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11815) == 1";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11858) == 1";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"checkquest(11852,HUNTING) == 2 && countitem(1000708) >= 10";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11829) == 2 && isbegin_quest(11851) == 0";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(11829) == 2 && isbegin_quest(11854) == 0 && checkquest(11859,PLAYTIME) == -1";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(11829) == 2 && isbegin_quest(11854) == 0 && checkquest(11859,PLAYTIME) == 2";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(11829) == 2 && isbegin_quest(11852) == 0 && checkquest(11853,PLAYTIME) == -1";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(11829) == 2 && isbegin_quest(11852) == 0 && checkquest(11853,PLAYTIME) == 2";
end;
}

icas_in,34,121,3	duplicate(dummynpc2)	Voglinde#ep19_room	4_EP19_VOGLINDE
icas_in,27,119,5	duplicate(dummynpc2)	Du#ep19_room	4_EP18_DEW
icas_in,30,121,5	duplicate(dummynpc2)	Mark#ep19_room	4_EP18_MARK
icas_in,29,120,5	duplicate(dummynpc2)	Maggi#ep19_room	4_4JOB_MAGGI
icas_in,26,119,5	duplicate(dummynpc2)	Alf#ep19_room	4_EP18_ALF

jor_back1,300,272,3	script	Hunting Trap#ep19_1	4_ROPEPILE,{
	if(isbegin_quest(11816) == 1){
		mes "- A cold-frozen hunting trap.";
		mes "It doesn't look like it has a problem other than it being old. -";
		next;
		mes "[Icewind Hunter]";
		mes "Who is it!";
		mes "I don't think you're a hunter, why are you touching our trap?";
		mes "An enemy?!";
		cutin "ep19_iwin09.png",2;
		next;
		select("Talk about the situation.");
		mes "[Icewind Hunter]";
		mes "I see.";
		mes "Pardon my actions.";
		next;
		mes "[Icewind Hunter]";
		mes "There's no problem around here.";
		mes "But I feel like my hunts is diminishing a bit...";
		next;
		mes "[Icewind Hunter]";
		mes "I think it's better to look further north from here.";
		mes "I think the monsters there have become more fierce recently...";
		completequest 11816;
		setquest 11817;
		close2;
		cutin "",255;
		navigateto("jor_back2",346,203);
		end;
	}
	if(isbegin_quest(11817) == 1){
		mes "- A cold-frozen hunting trap.";
		mes "It doesn't look like it has a problem other than it being old. -";
		next;
		mes "- Let's look further north -";
		close2;
		navigateto("jor_back2",346,203);
		end;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11816) == 1";
end;
}

jor_back2,346,203,3	script	Hunting Trap#ep19_2	4_ROPEPILE,{
	if(isbegin_quest(11817) == 1){
		mes "- A cold-frozen hunting trap.";
		mes "It doesn't look like it has a problem other than it being old. -";
		next;
		mes "- It would be better to investigate the area near the pit. -";
		completequest 11817;
		setquest 11818;
		close2;
		navigateto("jor_back2",260,294);
		end;
	}
	if(isbegin_quest(11818) == 1){
		mes "- A cold-frozen hunting trap.";
		mes "It doesn't look like it has a problem other than it being old. -";
		next;
		mes "- It would be better to investigate the area near the pit. -";
		close2;
		navigateto("jor_back2",260,294);
		end;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11817) == 1";
end;
}

jor_back2,260,294,3	script	Hunting Trap#ep19_3	4_ROPEPILE,{
	if(isbegin_quest(11818) == 1){
		mes "- A cold-frozen hunting trap.";
		mes "There are strange traces of decay. -";
		next;
		mes "[Alf]";
		mes "...Is it poison?";
		mes "I think it's similar to what was on Tamarin...";
		cutin "ep18_alf_01.png",2;
		cloaknpc("Alf#ep19_jor2",false,getcharid(0));
		cloaknpc("Mark#ep19_jor2",false,getcharid(0));
		next;
		mes "[Mark]";
		mes "Is there something in it?";
		mes "There's a monster that fell into the pit is trapped in or something like that...";
		cutin "ep18_mark_01.png",0;
		next;
		mes "[Alf]";
		mes "The connection is...";
		mes "There is a concentration of poison on it.";
		mes "It looks like someone was trying to ^E5555Edestroy it on purpose^000000.";
		cutin "ep18_alf_01.png",2;
		next;
		mes "[Alf]";
		mes "...Let's report to Syururu for now.";
		mes "I don't know who's destroying the trap, but I think it's related to the pit...";
		completequest 11818;
		setquest 11819;
		close2;
		cutin "",255;
		cloaknpc("Alf#ep19_jor2",true,getcharid(0));
		cloaknpc("Mark#ep19_jor2",true,getcharid(0));
		navigateto("jor_back2",251,310);
		end;
	}
	if(isbegin_quest(11819) == 1){
		mes "- A cold-frozen hunting trap.";
		mes "There are strange traces of decay.";
		mes "Let's report to Syururu. -";
		close;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11818) == 1";
end;
}


jor_back2,260,296,5	duplicate(dummynpc2)	Alf#ep19_jor2	4_EP18_ALF
jor_back2,258,295,3	duplicate(dummynpc2)	Mark#ep19_jor2	4_EP18_MARK

jor_back2,255,312,5	script	Syururu#ep19_1	4_EP19_IWIN_DIVER,{
	if(isbegin_quest(11819) == 1){
		mes "[Syururu]";
		mes "You're here.";
		mes "I think the poison in the pit is worse than I thought.";
		cutin "ep19_iwin_diver01.png",2;
		next;
		mes "[Syururu]";
		mes "I think my diving suit would last for some time, but ^E5555Eas soon as I entered the pit, my suit that is reinforced by a Mana Core broke^000000.";
		next;
		mes "[Mark]";
		mes "Well...";
		mes "I don't think further investigation would be meaningful.";
		mes "Let's go back and get ^E5555EFriederike^000000.";
		cutin "ep18_mark_01.png",0;
		cloaknpc("Mark#ep19_ab",false,getcharid(0));
		next;
		mes "[Mark]";
		mes "She purified up Tamarins decay earlier, she'll be able to purify this pit too.";
		next;
		mes "[Mark]";
		mes "If I knew this would happen, I should have brought her from the beginning.";
		cutin "ep18_mark_03.png",0;
		completequest 11819;
		setquest 11820;
		close2;
		cutin "",255;
		cloaknpc("Mark#ep19_ab",true,getcharid(0));
		navigateto("icas_in",32,120);
		end;
	}
	if(isbegin_quest(11820) == 1){
		mes "[Syururu]";
		mes "I think it's best to go to the Ice Castle and bring Lady ^E5555EFriederike^000000 here.";
		cutin "ep19_iwin_diver01.png",2;
		close2;
		cutin "",255;
		navigateto("icas_in",32,120);
		end;
	}
	if(isbegin_quest(11821) == 1){
		mes "[Syururu]";
		mes "Now that Lady Friederike is here, let's begin purifying the ^E5555Epit^000000!";
		cutin "ep19_iwin_diver01.png",2;
		close2;
		cutin "",255;
		navigateto("jor_back2",258,314);
		end;
	}
	if(isbegin_quest(11822) == 1){
		if(checkquest(11822,HUNTING) && countitem(1000708) >= 3){
			mes "[Syururu]";
			mes "Sigh...";
			mes "Maybe we've found something terrible...";
			cutin "ep19_iwin_diver01.png",2;
			delitem 1000708,3;
			completequest 11822;
			setquest 11823;
			next;
			mes "[Syururu]";
			mes "I dived into the pit, there wasa narrow passage that could barely fit on person, and it was blocked by a rock at the end.";
			mes "When I removed the rock, the water started to drain out and a showing the huge space behind it.";
			next;
			mes "[Syururu]";
			mes "I think I saw something like a ^E5555ERgan^000000, it seemed dangerous to investigate alone, so I cautiously ran away.";
			next;
			mes "[Syururu]";
			mes "I would like ^E5555Ego back inside^000000 and investigate.";
			mes "If this pit is really dangerous, we should let know Voglinde as quickly as possible.";
			close2;
			cutin "",255;
			navigateto("jor_back2",258,314);
			completequest 11823;
			setquest 11824;
		} else {
			mes "[Syururu]";
			mes "For the others, Please ^E5555Eclean up some monsters so it won't come near this place^000000.";
			mes "And if possible,  could you collect ^E5555E3 Frozen Meat^000000?";
			cutin "ep19_iwin_diver01.png",2;
			next;
			mes "[Syururu]";
			mes "I think we need to investigate if the monsters nearby are affected by this pit.";
			close2;
			cutin "",255;
		}
		end;
	}
	if(isbegin_quest(11823) == 1){
		mes "[Syururu]";
		mes "Sigh...";
		mes "Maybe we've found something terrible...";
		cutin "ep19_iwin_diver01.png",2;
		next;
		mes "[Syururu]";
		mes "I dived into the pit, there wasa narrow passage that could barely fit on person, and it was blocked by a rock at the end.";
		mes "When I removed the rock, the water started to drain out and a showing the huge space behind it.";
		next;
		mes "[Syururu]";
		mes "I think I saw something like a ^E5555ERgan^000000, it seemed dangerous to investigate alone, so I cautiously ran away.";
		next;
		mes "[Syururu]";
		mes "I would like ^E5555Ego back inside^000000 and investigate.";
		mes "If this pit is really dangerous, we should let know Voglinde as quickly as possible.";
		close2;
		cutin "",255;
		navigateto("jor_back2",258,314);
		completequest 11823;
		setquest 11824;
		end;
	}
	if(isbegin_quest(11824) == 1)
		warp "jor_ab01",113,10;
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11819) == 1";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"checkquest(11822,HUNTING) == 2 && countitem(1000708) >= 3";
end;
}

jor_back2,258,315,0	script	#to_jor_ab01	WARPNPC,1,1,{
	end;
	
OnTouch:
	if(isbegin_quest(11821) == 0){
		mes "- It's blocked by a murky and stickly liquid, and you cannot enter. -";
		close;
	}
	if(isbegin_quest(11821) == 1){
		mes "[Friederike]";
		mes "It's so cold...";
		cutin "ep19_friederike02.png",2;
		cloaknpc("Alf#ep19_ab",false,getcharid(0));
		cloaknpc("Mark#ep19_ab",false,getcharid(0));
		cloaknpc("Friederike#ep19_ab",false,getcharid(0));
		next;
		mes "[Friederike]";
		mes "I don't feel good here...";
		mes "The poison will rise again even if I purify everything around here.";
		mes "There's something deep down there that keeps spouting poison.";
		next;
		mes "[Syururu]";
		mes "It would be unreasonable to ask to purify everything around here.";
		mes "For now, let's purify up this part of the pit and investigate inside.";
		cutin "ep19_iwin_diver01.png",2;
		next;
		mes "[Friederike]";
		mes "The pit has already been purified...*Sneeze*!";
		cutin "ep19_friederike04.png",2;
		next;
		mes "[Mark]";
		mes "Oh!";
		mes "Then, we'll investigate later, you can go back now.";
		mes "I might end up in prison if you ever catch a cold.";
		cutin "ep18_mark_04.png",0;
		next;
		mes "[Friederike]";
		mes "Then, I'll teleport back.";
		cutin "ep19_friederike01.png",2;
		next;
		mes "[Friederike]";
		mes "Please attach these to your clothes, these are Mana cores that I purified earlier.";
		mes "Don't get hurt and come back.";
		cutin "ep19_friederike03.png",2;
		next;
		cloaknpc("Friederike#ep19_ab",true,getcharid(0));
		mes "[Syururu]";
		mes "I'm so glad that someone like Friederike came to the Ice Castle.";
		mes "She can prepare mana core easily like this...";
		cutin "ep19_iwin_diver01.png",2;
		next;
		mes "[Syururu]";
		mes "I can't believe that the pit that looks liked vomit turned into such clear water...";
		next;
		mes "[Syururu]";
		mes "...Anyway!";
		mes "It's not like I'll be doing this always, I'll go in first before the poison rise again.";
		next;
		mes "[Syururu]";
		mes "For the others, Please ^E5555Eclean up some monsters so it won't approach this place^000000.";
		mes "And if possible,  could you collect ^E5555E3 Frozen Meat^000000?";
		next;
		mes "[Syururu]";
		mes "I think we need to investigate if the monsters nearby are affected by this pit.";
		completequest 11821;
		setquest 11822;
		close2;
		cutin "",255;
		cloaknpc("Alf#ep19_ab",true,getcharid(0));
		cloaknpc("Mark#ep19_ab",true,getcharid(0));
		end;
	}
	if(isbegin_quest(11822) == 1){
		if(checkquest(11822,HUNTING) == 2 && countitem(1000708) >= 3){
			mes "- It seems that Syururu completed his investigation.";
			mes "Let's ^E5555Etalk to Syururu first^000000. -";
		} else
			mes "- The prevent monsters from approaching Syururu while he investigates, ^E5555Hunt 10 monsters in the area and collect 3 Frozen Meat^000000.";
		close;
	}
	if(isbegin_quest(11823) == 1){
		mes "- It seems that Syururu completed his investigation.";
		mes "Let's ^E5555Etalk to Syururu first^000000. -";
		close;
	}
	warp "jor_ab01",113,10;
	end;

OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11821) == 1";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11824) == 1";
end;
}

jor_back2,263,311,5	duplicate(dummynpc2)	Alf#ep19_ab	4_EP18_ALF
jor_back2,261,312,3	duplicate(dummynpc2)	Mark#ep19_ab	4_EP18_MARK
jor_back2,260,312,5	duplicate(dummynpc2)	Friederike#ep19_ab	4_EP19_FRIEDERIKE

jor_ab01,115,14,3	script	Alf#ep19_1_ab	4_EP18_ALF,{
	if(isbegin_quest(11824) == 1){
		mes "[Alf]";
		mes "What are all these?";
		mes "Snakes?";
		mes "Worms??";
		cutin "ep18_alf_01.png",2;
		next;
		mes "[Syururu]";
		mes "It's the race called Rgans.";
		mes "They're the scourge who serves Jormungandr.";
		cloaknpc("Syururu#ep19_ab",false,getcharid(0));
		cutin "ep19_iwin_diver01.png",2;
		next;
		mes "[Syururu]";
		mes "The snake nest should be a little deeper inside, but it suprises me to see that we already found a number of Rgans in a place like this.";
		mes "Let's go a bit deeper.";
		next;
		mes "[Syururu]";
		mes "We better not provoke the Rgans, but we should quietly ^E5555Ecatch and investigate^000000 a few of them.";
		cutin "ep19_iwin_diver02.png",2;
		completequest 11824;
		setquest 11825;
		close2;
		cloaknpc("Syururu#ep19_ab",true,getcharid(0));
		cutin "",255;
		navigateto("jor_ab01",222,107);
		end;
	}
	if(isbegin_quest(11825) == 1){
		mes "[Alf]";
		mes "What are all these?";
		mes "Snakes?";
		mes "Worms??";
		cutin "ep18_alf_01.png",2;
		next;
		mes "[Syururu]";
		mes "It's the race called Rgans.";
		mes "They're the scourge who serves Jormungandr.";
		cloaknpc("Syururu#ep19_ab",false,getcharid(0));
		cutin "ep19_iwin_diver01.png",2;
		next;
		mes "[Syururu]";
		mes "The snake nest should be a little deeper inside, but it suprises me to see that we already found a number of Rgans in a place like this.";
		mes "Let's go a bit deeper.";
		next;
		mes "[Syururu]";
		mes "We better not provoke the Rgans, but we should quietly ^E5555Ehunt and investigate^000000 a few of them.";
		cutin "ep19_iwin_diver02.png",2;
		close2;
		cloaknpc("Syururu#ep19_ab",true,getcharid(0));
		cutin "",255;
		navigateto("jor_ab01",222,107);
		end;
	}
	if(isbegin_quest(11849) == 1){
		mes "[Alf]";
		mes "Are you done?";
		mes "Then, return to the Ice Castle and report it directly to ^E5555ESyururu^000000.";
		cutin "ep18_alf_01.png",2;
		close2;
		cutin "",255;
		navigateto("icas_in",27,116);
		end;
	}
	if(isbegin_quest(11835) == 1 || isbegin_quest(11836) == 1){
		mes "[Alf]";
		mes "You're here...";
		mes "Then, I'll investigate this side, and you investigate the other places.";
		cutin "ep18_alf_01.png",2;
		next;
		mes "[Alf]";
		mes "After you finish your investigation, report it directly to ^E5555ESyururu^000000.";
		if(isbegin_quest(11835) == 1){
			completequest 11835;
			setquest 11836;
			for(.@i = 0; .@i < 4; .@i++){
				.@qid = rand(11837,11848);
				if(inarray(.@quest_id,.@qid) != -1)
					.@qid = rand(11837,11848);
				.@quest_id[.@i] = .@qid;
				setquest .@qid;
			}
		}
		close3;
	}
	mes "[Alf]";
	mes "...Ugh.";
	mes "This place is terrible.";
	cutin "ep18_alf_01.png",2;
	close3;
	
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11824) == 1";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11835) == 1";
end;
}

jor_ab01,163,253,3	script	Ominous Trail#ep19_1	4_ENERGY_BLACK,{
	if(isbegin_quest(11849) == 1){
		mes "- All investigations have been completed.";
		mes "Let's go back to Syururu. -";
		close;
	}
	if(isbegin_quest(11836) == 0) end;
	.@qid = 11836 + atoi(replacestr(strnpcinfo(2),"ep19_",""));
	if(isbegin_quest(.@qid) == 1){
		progressbar "B04A23",3;
		switch(rand(1,4)){
			case 1:
				unittalk getcharid(3),strcharinfo(0) + " : There's some suspicious handprint.",bc_self;
				break;
			case 2:
				unittalk getcharid(3),strcharinfo(0) + " : It's rotten to the core.",bc_self;
				break;
			case 3:
				unittalk getcharid(3),strcharinfo(0) + " : There's a trace of digging.",bc_self;
				break;
			case 4:
				unittalk getcharid(3),strcharinfo(0) + " : There's a trace of something...",bc_self;
				break;
		}
		erasequest .@qid;
		questinfo_refresh;
		for(.@i = 11837; .@i < 11849; .@i++)
			if(isbegin_quest(.@i) > 0) end;
		erasequest 11836;
		setquest 11849;
		questinfo_refresh;
	}
	end;
	
OnInit:
	.@qid = 11836 + atoi(replacestr(strnpcinfo(2),"ep19_",""));
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11836) == 1 && isbegin_quest(" + .@qid + ") == 1";
end;
}

jor_ab01,117,141,3	duplicate(Ominous Trail#ep19_1)	Ominous Trail#ep19_2	4_ENERGY_BLACK 
jor_ab01,222,125,3	duplicate(Ominous Trail#ep19_1)	Ominous Trail#ep19_3	4_ENERGY_BLACK 
jor_ab01,196,260,3	duplicate(Ominous Trail#ep19_1)	Ominous Trail#ep19_4	4_ENERGY_BLACK 
jor_ab01,112,62,3	duplicate(Ominous Trail#ep19_1)	Ominous Trail#ep19_5	4_ENERGY_BLACK 
jor_ab01,25,265,3	duplicate(Ominous Trail#ep19_1)	Ominous Trail#ep19_6	4_ENERGY_BLACK 
jor_ab02,151,272,3	duplicate(Ominous Trail#ep19_1)	Ominous Trail#ep19_7	4_ENERGY_BLACK 
jor_ab02,139,148,3	duplicate(Ominous Trail#ep19_1)	Ominous Trail#ep19_8	4_ENERGY_BLACK 
jor_ab02,182,83,3	duplicate(Ominous Trail#ep19_1)	Ominous Trail#ep19_9	4_ENERGY_BLACK
jor_ab02,48,27,3	duplicate(Ominous Trail#ep19_1)	Ominous Trail#ep19_10	4_ENERGY_BLACK
jor_ab02,208,48,3	duplicate(Ominous Trail#ep19_1)	Ominous Trail#ep19_11	4_ENERGY_BLACK
jor_ab02,201,184,3	duplicate(Ominous Trail#ep19_1)	Ominous Trail#ep19_12	4_ENERGY_BLACK

jor_ab01,116,11,3	duplicate(dummynpc2)	Syururu#ep19_ab	4_EP19_IWIN_DIVER

jor_ab01,223,107,3	script	Suspicious Traces#ep19_1_ab1	4_ENERGY_BLACK,{
	if(isbegin_quest(11825) == 1){
		if(checkquest(11825,HUNTING) < 2){
			mes "- It would be better to hunt down a few monsters around and investigate them. -";
			close;
		} else {
			mes "[Mark]";
			mes "Is the poison leaking out from this place?";
			cutin "ep18_mark_01.png",0;
			next;
			mes "[Alf]";
			mes "Hm...";
			mes "I think it's a decaying poison...";
			mes "I'm not sure if it's natural or an artificial...";
			cutin "ep18_alf_01.png",2;
			next;
			mes "[Syururu]";
			mes "The monsters around looks strange too.";
			mes "Let's move a little deeper while ^E5555Edealing with the monsters on our path^000000.";
			cutin "ep19_iwin_diver01.png",2;
			completequest 11825;
			setquest 11826;
			close2;
			cutin "",255;
			navigateto("jor_ab01",104,270);
		}
		end;
	}
	if(isbegin_quest(11826) == 1){
		mes "[Mark]";
		mes "Is the poison leaking out from this place?";
		cutin "ep18_mark_01.png",0;
		next;
		mes "[Alf]";
		mes "Hm...";
		mes "I think it's a decaying poison...";
		mes "I'm not sure if it's natural or an artificial...";
		cutin "ep18_alf_01.png",2;
		next;
		mes "[Syururu]";
		mes "The monsters around looks strange too.";
		mes "Let's move a little deeper while ^E5555Edealing with the monsters on our path^000000.";
		cutin "ep19_iwin_diver01.png",2;
		close2;
		cutin "",255;
		navigateto("jor_ab01",104,270);
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11825) == 1";
end;
}

jor_ab01,104,270,3	script	Suspicious Traces#ep19_1_ab2	4_ENERGY_BLACK,{
	if(isbegin_quest(11826) == 1){
		if(checkquest(11826,HUNTING) < 2){
			mes "- It would be better to hunt down a few monsters around and investigate them. -";
			close;
		} else {
			mes "[Alf]";
			mes "It's the same here...";
			mes "The poison are leaking out.";
			cutin "ep18_alf_01.png",2;
			next;
			mes "[Syururu]";
			mes "And also here, it's bigger than I thought.";
			mes "I don't this matter can be dealt easily at this scale.";
			cutin "ep19_iwin_diver01.png",2;
			next;
			mes "[Mark]";
			mes "Hm...";
			mes "This is a big problem.";
			mes "Everyone.";
			cutin "ep18_mark_04.png",0;
			next;
			mes "[Syururu]";
			mes "What's going on?";
			cutin "ep19_iwin_diver01.png",2;
			next;
			mes "[Mark]";
			mes "There's a passage insde that looks like it's connected to something.";
			cutin "ep18_mark_04.png",0;
			next;
			mes "[Syururu]";
			mes "Isn't this the end?!";
			mes "Let's go ahead.";
			mes "We need to get information about the variation of monsters too.";
			mes "So as we go, ^E555EDdefeat some monsters and collect Frozen Meat samples^000000.";
			cutin "ep19_iwin_diver01.png",2;
			next;
			mes "[Mark]";
			mes "That's a good idea.";
			mes "This way, follow me.";
			cutin "ep18_mark_04.png",0;
			completequest 11826;
			setquest 11827;
			close2;
			cutin "",255;
			navigateto("jor_ab01",16,235);
		}
		end;
	}
	if(isbegin_quest(11827) == 1){
		mes "[Syururu]";
		mes "Isn't this the end?!";
		mes "Let's go ahead.";
		mes "We need to get information about the variation of monsters too.";
		mes "So as we go, ^E555Edefeat some monsters and collect Frozen Meat samples^000000.";
		cutin "ep19_iwin_diver01.png",2;
		next;
		mes "[Mark]";
		mes "That's a good idea.";
		mes "This way, follow me.";
		cutin "ep18_mark_04.png",0;
		close2;
		cutin "",255;
		navigateto("jor_ab01",16,235);
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11826) == 1";
end;
}

jor_ab01,8,238,0	script	#to_jor_ab02	WARPNPC,1,1,{
	end;
	
OnTouch:
	if(isbegin_quest(11827) == 1){
		if(checkquest(11827,HUNTING) == 2 && countitem(1000708) >= 2){
			mes "[Mark]";
			mes "It's over here.";
			mes "Can you see it?";
			mes "This passage...";
			cutin "ep18_mark_01.png",0;
			delitem 1000708,2;
			completequest 11827;
			setquest 11828;
			next;
			mes "[Mark]";
			mes "This...";
			mes "It's not as much as it was at the entrance, but it seems that poison is accumulating little by little..";
			mes "The Mana Core seems almost at its limit and going further will be dangerous, so let's quickly investigate down there.";
			next;
			cutin "ep19_iwin_diver01.png",2;
			mes "[Syururu]";
			mes "^E5555EDefeat some monsters and collect Frozen Meat samples^000000 and meet us at the end of the passage.";
			close2;
			cutin "",255;
			navigateto("jor_ab02",28,253);
			warp "jor_ab02",281,87;
			end;
		}
	}
	if(isbegin_quest(11828) > 0)
		warp "jor_ab02",281,87;
end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"checkquest(11827,HUNTING) == 2 && countitem(1000708) >= 2";
end;
}

jor_ab02,28,254,3	script	End of Pit#ep19_1_ab	4_ENERGY_BLACK,{
	if(isbegin_quest(11828) == 1){
		if(checkquest(11828,HUNTING) == 2 && countitem(1000708) >= 2){
			mes "[Syururu]";
			mes "I think this is the end.";
			mes "I don't think there's anything special except from the monsters that I haven't seen before, did you find anything else?";
			cutin "ep19_iwin_diver01.png",2;
			delitem 1000708,2;
			completequest 11828;
			setquest 11829;
			next;
			mes "[Mark]";
			mes "Fortunately, I don't think there's anymore passages that connects to another place.";
			cutin "ep18_mark_01.png",0;
			next;
			mes "[Alf]";
			mes "...";
			cutin "ep18_alf_01.png",2;
			next;
			mes "[Syururu]";
			mes "If this is the end, I'm glad...";
			cutin "ep19_iwin_diver01.png",2;
			next;
			mes "[Syururu]";
			mes "Then let's go back and report to Lehar.";
			mes "I don't even want to be here anymore.";
			next;
			mes "[Mark]";
			mes "I think I feel a little sick too...";
			mes "Let's go back.";
			cutin "ep18_mark_01.png",0;
			close2;
			cutin "",255;
			navigateto("icas_in",32,120);
		}
		end;
	}
	if(isbegin_quest(11829) == 1){
		cutin "ep19_iwin_diver01.png",2;
		mes "[Syururu]";
		mes "Then let's go back and report to Lehar.";
		mes "I don't even want to be here anymore.";
		next;
		mes "[Mark]";
		mes "I think I feel a little sick too...";
		mes "Let's go back.";
		cutin "ep18_mark_01.png",0;
		close2;
		cutin "",255;
		navigateto("icas_in",32,120);
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"checkquest(11828,HUNTING) == 2 && countitem(1000708) >= 2";
end;
}

jor_back2,367,36,3	script	Frozen Trap#ep19_1	4_ROPEPILE,{
	if(isbegin_quest(11854) == 1){
		mes "- A cold-frozen hunting trap.";
		mes "It's very old but it can still be used if it's maintained well. -";
		next;
		progressbar "FFFF00",3;
		mes "[" + strcharinfo(0) + "]";
		mes "I wish it'll hold out...";
		next;
		mes "- The maintenance has been completed.";
		mes "Let's go to the next trap. -";
		completequest 11854;
		setquest 11855;
		questinfo_refresh;
		close2;
		navigateto("jor_back2",84,88);
		end;
	}
	if(isbegin_quest(11855) > 0){
		mes "- The maintenance has been completed.";
		mes "Let's go to the next trap. -";
		close2;
		navigateto("jor_back2",84,88);
		end;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11854) == 1";
end;
}

jor_back2,84,88,3	script	Frozen Trap#ep19_2	4_ROPEPILE,{
	if(isbegin_quest(11855) == 1){
		mes "- A cold-frozen hunting trap.";
		mes "It's very old but it can still be used if it's maintained well. -";
		next;
		progressbar "FFFF00",3;
		mes "[" + strcharinfo(0) + "]";
		mes "I wish it'll hold out...";
		next;
		mes "- The maintenance has been completed.";
		mes "Let's go to the next trap. -";
		completequest 11855;
		setquest 11856;
		questinfo_refresh;
		close;
		navigateto("jor_back2",158,320);
		end;
	}
	
	if(isbegin_quest(11856) > 0){
		mes "- The maintenance has been completed.";
		mes "Let's go to the next trap. -";
		close;
		navigateto("jor_back2",158,320);
		end;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11855) == 1";
end;
}

jor_back2,158,320,3	script	Frozen Trap#ep19_3	4_ROPEPILE,{
	if(isbegin_quest(11856) == 1){
		mes "- A cold-frozen hunting trap.";
		mes "It's very old but it can still be used if it's maintained well. -";
		next;
		progressbar "FFFF00",3;
		mes "[" + strcharinfo(0) + "]";
		mes "I wish it'll hold out...";
		next;
		mes "- The maintenance has been completed.";
		mes "Let's go to the next trap. -";
		completequest 11856;
		setquest 11857;
		questinfo_refresh;
		close2;
		navigateto("jor_back2",190,184);
		end;
	}
	if(isbegin_quest(11857) > 0){
		mes "- The maintenance has been completed.";
		mes "Let's go to the next trap. -";
		close2;
		navigateto("jor_back2",190,184);
		end;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11856) == 1";
end;
}

jor_back2,190,184,3	script	Frozen Trap#ep19_4	4_ROPEPILE,{
	if(isbegin_quest(11857) == 1){
		mes "- A cold-frozen hunting trap.";
		mes "It's very old but it can still be used if it's maintained well. -";
		next;
		progressbar "FFFF00",3;
		mes "[" + strcharinfo(0) + "]";
		mes "I wish it'll hold out...";
		next;
		mes "- The maintenance has been completed.";
		mes "Let's go report back to Tatarin. -";
		completequest 11857;
		setquest 11858;
		close2;
		navigateto("icas_in",28,119);
		end;
	}
	if(isbegin_quest(11858) > 0){
		mes "- The maintenance has been completed.";
		mes "Let's go report back to Tatarin. -";
		close2;
		navigateto("icas_in",28,119);
		end;
	}
	end;

OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(11857) == 1";
end;
}

jor_tail,48,241,3	script	Moryara#0kb20	4_EP19_MORYARA,{
	if(isbegin_quest(18126) < 2){
		mes "[Moryara]";
		mes "I've heard that humans are coming...";
		mes "Did you get the permission to go out? Acting alone without permission is very dangerous.";
		mes "Return to the nest.";
		close;
	}
	if(isbegin_quest(8784) == 0){
		mes "[Moryara]";
		mes "...";
		mes "Shh.";
		mes "The prey. It will run away.";
		next;
		mes "The Iwin who is fishing without turning it's head said it quietly as I approached.";
		next;
		mes "[Moryara]";
		mes "Humans, no matter how small you are, you are bigger than the a fish, so please walk gently around here.";
		mes "I'm fishing right now.";
		next;
		select("Is small fish edible?:Are you good at fishing?");
		mes "[Moryara]";
		mes "The preparation for different kinds of fish is important, even if it's not big everytime.";
		mes "Just to make the table more colorful.";
		mes "Human, are you good at fishing?";
		next;
		select("I'm decent.:I've done it before.:A little.");
		mes "[" + strcharinfo(0) + "]";
		mes "As an adventurer, there are times that I have to do these kind of things.";
		next;
		mes "[Moryara]";
		mes "Ho...";
		mes "Then you must have tied a bait before. Can you help me with this for a while?";
		next;
		mes "The Iwin turned around and pointed at something.";
		next;
		mes "[Moryara]";
		mes "Now, put the bait on this side of the hook on the fishing rod that hasn't been set up yet...";
		next;
		mes "[Moryara]";
		mes "Like this...";
		mes "Like that...";
		next;
		mes "[Moryara]";
		mes "...";
		next;
		mes "It's more interesting than I thought.";
		mes "It's a perfect place to look at the cold sea and organize your thoughts.";
		next;
		mes "[Moryara]";
		mes "...!";
		next;
		mes "The Iwin moved his larged body with great agility and pulled the fishing rod.";
		next;
		mes "[Moryara]";
		mes "Oh... I've caught a big one today.";
		mes "It's all thanks to you human who helped me.";
		mes "Why don't you try fishing, human?";
		next;
		select("I'll try once.");
		mes "[Moryara]";
		mes "Then, I'll lend you this fishing rod. Just thread the bait again and throw it.";
		next;
		mes "As the Iwin told me, I thread the bait on the line and cast it on the sea.";
		next;
		mes "As the Iwin told me, I thread the bait on the line and cast it on the sea.";
		mes "...";
		next;
		mes "As the Iwin told me, I thread the bait on the line and cast it on the sea.";
		mes "...";
		mes "......";
		next;
		mes "As the Iwin told me, I thread the bait on the line and cast it on the sea.";
		mes "...";
		mes "......";
		mes ".........";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "!";
		next;
		mes "[Moryara]";
		mes "Oh! This is a... Seaweed.";
		mes "It will be delicious. I have a talent for this.";
		next;
		mes "[Moryara]";
		mes "It will help hugely to our food stock!";
		mes "I know you're busy, but can you lend me a hand more?";
		next;
		select("I don't think I'm good at fishing.");
		mes "[" + strcharinfo(0) + "]";
		mes "...I think I'm better at hunting than fishing.";
		next;
		mes "[Moryara]";
		mes "Oh! Of course, just waiting to catch it from the water is not enough. As expected from a warrior with such a different mindset.";
		mes "Then around this place...";
		mes "Shining Seaweeds are growing naturally. Can you catch me 20 of it?";
		next;
		if(select("Accept.:I'm busy.") == 2){
			mes "[Moryara]";
			mes "There's nothing I can do since you're busy.";
			mes "Ah... Not everyone is born with a talent to catch seaweeds...";
			close;
		}
		mes "[Moryara]";
		mes "Thank you very much!";
		mes "They sound cute, but they don't do cute things at all. Be careful.";
		setquest 8784;
		close;
	}
	if(isbegin_quest(8784) == 1){
		if(checkquest(8784,HUNTING) < 2){
			mes "[Moryara]";
			mes "It grows naturally on this area.";
			mes "Please hunt 20 ^0000FFShining Seaweed^000000.";
			close;
		}
		mes "[Moryara]";
		mes "Oh, you're back.";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "..." + strcharinfo(0) + ".";
		mes "Call me by my name.";
		next;
		mes "[Moryara]";
		mes "Ohohoh, " + strcharinfo(0) + "!";
		mes "You're name is nice.";
		mes "You can call me Moryara, but the pronunciation that is a little easier for humans is, Asha Scarlet Baby Gracia Cassia Carena Eva Aloha.";
		mes "Call me on what you're comfortable with.";
		next;
		mes "[Moryara]";
		mes "Have you caught the Shining Seaweed?";
		mes "I'm grateful for this. As you can see, I can only fish. The kids on the nest will love this.";
		mes "If you have the time, how about fishing with me again tomorrow?";
		completequest 8784;
		setquest 8785;
		addreputation("Ice Castle",5);
		getitem 1000608,2 + (getreputation("Ice Castle") >= 3000 ? 1 : 0);
		close;
	}
	switch(checkquest(8785,PLAYTIME)){
		case -1:
		
		case 0:
		case 1:
			mes "[Moryara]";
			mes "The seaweed is enough for today.";
			mes "The children will love this.";
			close;
		
		case 2:
			erasequest 8785;
			break;
	}
	if(isbegin_quest(8786) == 0){
		mes "[Moryara]";
		strcharinfo(0) + ", you're back?";
		mes "How about fishing again today.";
		mes "... is it fine?";
		mes "If you have the time, Do you think you can bring me 20 of ^0000FFShining Seaweed^000000 that grows wild around here?";
		next;
		if(select("Accept.:I'm busy.") == 2){
			mes "[Moryara]";
			mes "There's nothing I can do since you're busy.";
			mes "Even fishing...";
			mes "No. No.";
			close;
		}
		mes "[Moryara]";
		mes "Thank you very much!";
		mes "They sound cute, but they don't do cute things at all. Be careful.";
		setquest 8786;
		close;
	}
	if(isbegin_quest(8786) == 1){
		if(checkquest(8786,HUNTING) < 2){
			mes "[Moryara]";
			mes "It grows naturally on this area.";
			mes "Please hunt 20 ^0000FFShining Seaweed^000000.";
			close;
		}
		mes "[Moryara]";
		mes "Oh, you're back.";
		next;
		mes "[Moryara]";
		mes "Thank you for today.";
		mes "These amount should be enough for today, you should take a good rest.";
		mes "I'll need you energy again tomorrow.";
		erasequest 8786;
		setquest 8785;
		addreputation("Ice Castle",5);
		getitem 1000608,2 + (getreputation("Ice Castle") >= 3000 ? 1 : 0);
		close;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(18126) == 2 && isbegin_quest(8784) == 0";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"checkquest(8784,HUNTING) == 2";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(8784) == 2 && isbegin_quest(8786) == 0 && checkquest(8785,PLAYTIME) == -1";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(8784) == 2 && isbegin_quest(8786) == 0 && checkquest(8785,PLAYTIME) == 2";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"checkquest(8786,HUNTING) == 2";
end;
}

icecastle,23,115,8	script	Patrol Leader#iws	21518,{
	if(isbegin_quest(18127) == 0){
		mes "[" + strcharinfo(0) + "]";
		mes "Eagle patrol? What does your unit do?";
		next;
		mes "[Patrol Leader]";
		mes "Our unit is responsible fot patrolling areas that are frozen.";
		next;
		mes "[Patrol Leader]";
		mes "We're only made up of Iwins, and we're the only one that are in charge of patrolling.. ";
		next;
		mes "[Patrol Leader]";
		mes "Adventurers have already began to flow into the Ice Castle of Isgard.";
		next;
		mes "[Patrol Leader]";
		mes "With more people like you, wouldn't we have a chance to patrol together and form a harmonious relationship?";
		next;
		mes "[Patrol Leader]";
		mes "Hahaha! I'm looking forward to that day.";
		close;
	}
	if(isbegin_quest(18127) == 1){
		mes "[Patrol Leader]";
		mes "Did you come here after seeing the announcement? Huh? No?";
		next;
		mes "[Patrol Leader]";
		mes "Anyway, you've come to the right place.";
		mes "I am Marsha Gigi Happy Rev Eve Alice Lloyd Broad-Minded Tess Vortex, The captain of the Eagle Patrol.";
		next;
		cloaknpc("#iwsmember",false,getcharid(0));
		npctalk "Horiryu : Reporting to the Eagle Patrol captain.","#iwsmember",bc_self;
		sleep2 1000;
		npctalk "Patrol Leader: Is there anything wrong?","Patrol Leader#iws",bc_self;
		sleep2 1000;
		npctalk "Horiryu : Yes, everything is clear.","#iwsmember",bc_self;
		sleep2 1000;
		npctalk "Horiryu : Eh?! Is this one of the adventurer who came to the Ice Castle this time?","#iwsmember",bc_self;
		sleep2 1000;
		npctalk "Patrol Leader: Oh! Come to think of it, I also heard stories like that. Anyway, good job. Get some rest.","Patrol Leader#iws",bc_self;
		sleep2 1000;
		npctalk "Horiryu : Yes.","#iwsmember",bc_self;
		cloaknpc("#iwsmember",true,getcharid(0));
		mes "[Patrol Leader]";
		mes "Sorry to keep you waiting. These are precious people, it's more important to receive their reports.";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "Is it Horyoryo?";
		next;
		npctalk "(Tsk! These outsiders..)","Patrol Leader#iws",bc_self;
		mes "[Patrol Leader]";
		mes "Hmm.. Well, you're not an official member of the patrol, so you can call me whatever you're comfortable with. Here's the Eagle Patrol membership document.";
		next;
		mes "[Patrol Leader]";
		mes "If you really want to sign up. You need to follow procedures. Please read it carefully and sign it.";
		next;
		mes "[Eagle Patrol Membership Document]";
		mes "Are you a Rgan?";
		mes "Is your body healthy?";
		mes "Are you peaceful?";
		mes "If there's no urgent situation, would you join the patrol everyday?";
		mes "----";
		mes "If you agree with the above, sign here.";
		next;
		select("Sign.");
		mes "[Patrol Leader]";
		mes "Did you practice this a lot? It's a nice signature. You'll be named as an honorary member of the Eagle Patrol.";
		next;
		completequest 18127;
		setquest 5972;
		mes "[Patrol Leader]";
		mes "You can do it alone or with your colleagues. Make a party and tell me when you're ready to patrol.";
		close;
	}
	.@md_name$ = "Iwin Patrol";
	switch(checkquest(5973,PLAYTIME)){
		case -1:
			break;
			
		case 0:
		case 1:
		
		case 2:
			.@name$ = instance_live_info(ILI_NAME,instance_id(IM_PARTY));
			mes "[Patrol Leader]";
			mes "Haven't you been in a patrol already? What's going on?";
			next;
			switch(select((.@name$ == .@md_name$ ? "Leave for the Patrol.":""),"Just want to chat.","I just came to say hello.")){
				case 1:
					if(instance_enter(.@md_name$) == IE_OK){
						setquest 5974;
						mapannounce "icecastle",strcharinfo(0) + " : I will be going on a patrol.", bc_map, "0x0DF297";
					} else {
						mes "[Patrol Leader]";
						mes "We are done for today."; //TODO
						close;
					}
					end;
					
				case 2:
					mes "[" + strcharinfo(0) + "]";
					mes "What do the Eagle Patrol usually do?";
					next;
					mes "[Patrol Leader]";
					mes "We are watching the area to see if a Rgan came in.";
					next;
					mes "[Patrol Leader]";
					mes "That's we patrol the Frozen Scale Hill.";
					next;
					mes "[Patrol Leader]";
					mes "I'm grateful that accidents have been greatly reduced since you've joined the patrol. I hope you'll keep joining us.";
					close;
				
				case 3:
					mes "[Patrol Leader]";
					mes "You're quite bored, friend. If you're bored, go around the castle. But you wouldn't want to move too much since it's cold.";
					close;
			}
			end;
	}
	if(isbegin_quest(5972) == 1){
		mes "[Patrol Leader]";
		mes "Your first patrol? You don't have to be afraid since I won't be sending you alone.";
	} else {
		mes "[Patrol Leader]";
		mes "Are you going out for the patrol again today?"; //TODO
	}
	next;
	.@party_id = getcharid(0);
	switch(select((is_party_leader()?"Register as a patrol personnel.":""),"I'll go on a patrol.","Please wait a moment.")){
		case 1:
			if(!.@party_id){
				mes "[Patrol Leader]";
				mes "Didn't I tell you to make a party even if you're going alone? Have you not done it? Now that you're here, go and make a party.";
				close;
			}
			getpartymember .@party_id,1;
			if(instance_create(.@md_name$) == -3) 
				instance_warning(2,.@md_name$);
			mes "[Patrol Leader]";
			mes "I have registered " + ($@partymembercount + 1) + " people from the party. It's not necessary for all the registered people from the party to attend the patrol, but before we go out on the patrol, we will check the number of person, to make sure of the participants.";
			close;
			
		case 2:
			if(!.@party_id){
				mes "[Patrol Leader]";
				mes "Didn't I tell you to make a party even if you're going alone? Have you not done it? Now that you're here, go and make a party.";
				close;
			}
			if(instance_enter(.@md_name$) == IE_OK){
				if(!isbegin_quest(5974)) setquest 5974;
				mapannounce "icecastle",strcharinfo(0) + " : I will be going on a patrol.", bc_map, "0x0DF297";
			} else {
				mes "[Patrol Leader]";
				mes "The unit is still not ready, let's wait a little bit.";
				close;
			}
			break;
			
		case 3:
			mes "[Patrol Leader]";
			mes "The patrol is not urgent, so take you're time. If you're really busy, you can just send your party.";
			close;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(18127) == 1";
	questinfo QTYPE_CLICKME,QMARK_YELLOW,"isbegin_quest(18127) == 1 && checkquest(5973,PLAYTIME) == -1";
	questinfo QTYPE_CLICKME,QMARK_YELLOW,"isbegin_quest(18127) == 1 && checkquest(5973,PLAYTIME) == 2";
	setunittitle getnpcid(0),"<Eagle Patrol>";
end;
}

icecastle,20,118,5	duplicate(dummynpc2)	#iwsmember	21517


jor_back2,250,30,0	script	#lunchtrigger	HIDDEN_WARP_NPC,20,20,{
	end;
	
OnTouch:
	if(isbegin_quest(5983) == 1){
		cloaknpc("Mysterious Young Man#flunch",false,getcharid(0));
		unittalk getcharid(3),strcharinfo(0) + " : Huh?! Is that the young man?",bc_self;
	}
	if(isbegin_quest(5983) == 2)
		cloaknpc("Lunch#flunch",false,getcharid(0));
end;
}

jor_back2,250,30,2	script	Mysterious Young Man#flunch	4_EP19_LUNCH,{
	if(isbegin_quest(5983) == 1){
		cutin "ep19_lunch01.png",2;
		mes "[Mysterious Young Man]";
		mes "Huh?! It's you again!";
		next;
		mes "[" + strcharinfo(0) +"]";
		mes "Oh! It's you, my test subject friend!";
		next;
		mes "[Mysterious Young Man]";
		mes "Oh! It's my first time to be called as a friend. Friend.. a friend is good.";
		next;
		mes "[" + strcharinfo(0) +"]";
		mes "Oh! Have you ever mentioned me your name? Introduce yourself. What's your name? What should I call you?";
		next;
		mes "[Mysterious Young Man]";
		mes "Name?";
		next;
		mes "[Mysterious Young Man]";
		mes "Oh! Are you talking about what to call me? If that's the case, call me HU-210426.";
		next;
		mes "[Mysterious Young Man]";
		mes "You'll find out my name if you scan the barcode on my head..";
		next;
		mes "[Mysterious Young Man]";
		mes "But you're not a Illusion scientist, so you don't have that kind of equipment. I'm relieve that you're not.";
		next;
		mes "[" + strcharinfo(0) +"]";
		mes "Oh! My apologies, I forgot that you're an experiment. But HU-210426 is hard to call.";
		next;
		mes "[" + strcharinfo(0) +"]";
		mes "Lunch! how about lunch? It's lunch time right now, and it's way easier to call compare to HU-210426.";
		next;
		cloaknpc("Mysterious Young Man#flunch",true,getcharid(0));
		cloaknpc("Lunch#flunch",false,getcharid(0));
		mes "[Mysterious Young Man]";
		mes "Lunch.. lunch..";
		mes "That's a good nickname. Then from now on, please call me lunch.";
		next;
		mes "[" + strcharinfo(0) +"]";
		mes "Yes, Lunch! It might be painful to remember but, can you tell me how you managed to escape?";
		next;
		mes "[Lunch]";
		mes "It is painful. Where should I start? Oh! I can start from that part.";
		next;
		mes "[Lunch]";
		mes "It was on a  day when the scientists were experimenting with the usual things in the big mansion.";
		next;
		mes "[Lunch]";
		mes "Then, all of a sudden the scientists started packing their things.";
		next;
		mes "[Lunch]";
		mes "There were many experiments that were abandoned and discarded in the process, as for me the scientists brought me in an airship.";
		next;
		mes "[" + strcharinfo(0) +"]";
		mes "It must have been hard?";
		next;
		mes "[Lunch]";
		mes "It was hard. It's common for the experiments to be abandoned or discarded. Then, I'll continue.";
		next;
		mes "[Lunch]";
		mes "When the scientist came here, they met a long-bodied beings. The race called Rgans.";
		next;
		mes "[Lunch]";
		mes "I don't know what the scientists are doing with those race, but they suddenly were not interested on the experiments that they brought from the mansion.";
		next;
		mes "[Lunch]";
		mes "No, it's not that they were not interested, but it felt like they were distracted by the other experiments and forgot about our existence.";
		next;
		mes "[Lunch]";
		mes "Well, I think it's common for scientists to be obsessed with experiments.";
		next;
		mes "[Lunch]";
		mes "The other neglected experiments collapsed one by one. As for me, I'm an individual who could eat anything to produce energy, but..";
		next;
		mes "[Lunch]";
		mes "the other experiments are not.";
		next;
		mes "[Lunch]";
		mes "After a while, I fell asleep until I was the only experiment that remained..";
		next;
		mes "[Lunch]";
		mes "I was bored and.. I turned into a small animal that passed by and escaped the cage where I was trapped in.";
		next;
		mes "[" + strcharinfo(0) +"]";
		mes "What? You turned into a small animal?";
		next;
		mes "[Lunch]";
		mes "Oh! One of my abilities is to transform into another being.";
		next;
		mes "[Lunch]";
		mes "Isn't it amazing?";
		mes "It's my personal advanced ability that the early experiments can't do! The scientists probably brought me all the way here because of this ability.";
		next;
		mes "[" + strcharinfo(0) +"]";
		mes "Could you also transform others into another being?";
		next;
		mes "[Lunch]";
		mes "Hm.. Unlike me, I will need some materials.. Not everyone can be transformed but..";
		next;
		mes "[Lunch]";
		mes "it's not impossible. For example, the Rgan race? Anyone can transform into something simple like that.";
		next;
		mes "[" + strcharinfo(0) +"]";
		mes "Okay! Can you make one for me?";
		next;
		mes "[Lunch]";
		mes "If you can prepare the materials. 2 Rgan's Low-Grade Mana Core and 2000 Zeny?";
		next;
		mes "[" + strcharinfo(0) +"]";
		mes "Zeny? Are you talking about money?";
		next;
		mes "[Lunch]";
		mes "Yes!! I've heard that you can live in Prontera without discrimination if you have abilities.";
		next;
		mes "[Lunch]";
		mes "I'm going to save a lot of Zeny and buy a house in Prontera and live the rest of my life there!";
		next;
		mes "[" + strcharinfo(0) +"]";
		mes "It'll be hard, but I'm rooting for you..";
		next;
		mes "[Lunch]";
		mes "Oh! My friend, I have a favor to ask.";
		next;
		mes "[" + strcharinfo(0) +"]";
		mes "A favor? What favor?";
		next;
		mes "[Lunch]";
		mes "I'll be staying here for a while, but there's very little to eat here to use as energy.";
		next;
		mes "[Lunch]";
		mes "Ice has a very low energy efficiency.. Friend, could you prepare a meal for me?";
		next;
		mes "[" + strcharinfo(0) +"]";
		mes "That's easy.";
		next;
		mes "[Lunch]";
		mes "Hehe, I knew that my friend who came back would do me a favor.";
		next;
		completequest 5983;
		setquest 5984;
		setquest 5985;
		getitem 101162,1;
		getitem 1000608,2;
		mes "[Lunch]";
		mes "This is a gift for you my friend. Please take good care of me starting tomorrow.";
		close2;
		cutin "",255;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(5983) == 1";
end;
}

jor_back2,250,30,2	script	Lunch#flunch	4_EP19_LUNCH,{
	if(isbegin_quest(5983) < 2) end;
	cutin "ep19_lunch01.png",2;
	if(checkquest(5985,PLAYTIME) == 0 || checkquest(5985,PLAYTIME) == 1)
		.@quest$ = "^777777[next;] Bring Food for Lunch^000000";
	else {
		setarray .@qid,5986,5987,5988,5989,5990,5991,5992,5993,5994,5995;
		setarray .@item_id,7158,7162,7299,7266,7762,7171,6405,6498,25285,7150;
		.@size = getarraysize(.@qid);
		for(.@i = 0; .@i < .@size; .@i++){
			if(isbegin_quest(.@qid[.@i])){
				if(countitem(.@item_id[.@i]) >= 10){
					npctalk "Lunch : Oh!","",bc_self;
					cutin "ep19_lunch02.png",2;
					sleep2 1000;
					erasequest .@qid[.@i];
					delitem .@item_id[.@i],10;
					setquest 5985;
					getitem 101162,1;
					getitem 1000608,2;
					sleep2 1000;
					npctalk "Lunch : Thank you for taking care of my energy source. You will be blessed.","",bc_self;
					cutin "",255;
					end;
				}
				.@quest_id = .@qid[.@i];
				.@quest$ = "^777777[Ongoing] Bring Food for Lunch^000000";
				break;
			}
		}
	}
	npctalk "Lunch : - - · · 　· 　- - 　- - · 　- - · - 　· - · · 　　　· - - · 　· · · · 　- · -","",bc_self;
	sleep2 1000;
	emotion ET_HUK;
	npctalk "Lunch : Oh! You're here?","",bc_self;
	if(!.@quest_id) .@quest$ = "Bring Food for Lunch";
	switch(select(.@quest$,"Make a Transformation Scroll..","Chat with Lunch.")){
		case 1:
			switch(checkquest(5985,PLAYTIME)){
				case -1:
					break;
					
				case 0:
				case 1:
					npctalk "Lunch : I heard that other beings eat three meals a day, but..","",bc_self;
					sleep2 1000;
					npctalk "Lunch : I'm an efficient experiment.","",bc_self;
					sleep2 1000;
					npctalk "Lunch : Intaking an energy source once a day is enough. I appreciate the thought.","",bc_self;
					cutin "",255;
					end;
					
				case 2:
					erasequest 5985;
					break;
			}
			setarray .@qid,5986,5987,5988,5989,5990,5991,5992,5993,5994,5995;
			setarray .@item_id,7158,7162,7299,7266,7762,7171,6405,6498,25285,7150;
			.@size = getarraysize(.@qid);
			for(.@i = 0; .@i < .@size; .@i++){
				if(isbegin_quest(.@qid[.@i])){
					if(countitem(.@item_id[.@i]) < 10){
						npctalk "Lunch : Have you brought the " + getitemname(.@item_id[.@i]) + " that I asked for?","",bc_self;
						sleep2 1000;
						npctalk "Lunch : If I convert to energy-saving mode, last a year would be easy, but I might lose some data.","",bc_self;
						sleep2 1000;
						npctalk "Lunch : I really don't want that to happen.","",bc_self;
						cutin "",255;
						end;
					}
				}
			}
			.@index = rand(.@size);
			npctalk "Lunch: Let's see.. " + getitemname(.@item_id[.@index]) + " seem to have the best energy conversion efficiency.","",bc_self;
			sleep2 1000;
			npctalk "Lunch : I think 10 pieces will be enough.","",bc_self;
			if(select("I'll bring it.:Something urgent came up.") == 2){
				cutin "ep19_lunch03.png",2;
				npctalk "Lunch : Oh! You seem busy.","",bc_self;
				sleep2 1000;
				npctalk "Lunch : Then, I'll figure out a way to get my own energy source.","",bc_self;
				cutin "",255;
				end;
			}
			unittalk getcharid(3),strcharinfo(0) + " : Lunch, just wait for a while, I'll bring it to you.",bc_self;
			setquest .@qid[.@index];
			sleep2 1000;
			cutin "ep19_lunch02.png",2;
			npctalk "Lunch : I'll be waiting for you then.","",bc_self;
			sleep2 1000;
			cutin "",255;
			end;
		
		case 2:
			npctalk "Lunch : Sure, I'll make it for you. How many do you need?","",bc_self;
			cutin "",255;
			callshop "Ep19_Rgan_Scroll",1;
			end;
			
		case 3:
			unittalk getcharid(3),strcharinfo(0) + " : How do you make a Rgan Transformation Scroll?",bc_self;
			sleep2 1000;
			npctalk "Lunch : Oh! It's about that. It's not difficult. It's an object that contains the mana of the target to transform into..","",bc_self;
			sleep2 1000;
			npctalk "Lunch : To make a Rgan Transformation Scroll, I need to put ten Rgan's Low-Grade Mana Core in my mouth..","",bc_self;
			sleep2 1000;
			npctalk "Lunch : Then I need to chew it- then, If I do this and-that- my saliva will turn the mana core into a thick material.","",bc_self;
			sleep2 1000;
			emotion ET_BEST;
			npctalk "Lunch : Then, I'll dab that material on a paper to make it.","",bc_self;
			sleep2 1000;
			unittalk getcharid(3),strcharinfo(0) + " : I.. I see. I shouldn't have asked..",bc_self;
			cutin "",255;
			end;
	}
	end;
	
OnInit:
	.@cid$ = "checkquest(5985,PLAYTIME) == -1 && ";
	.@cid2$ = "checkquest(5985,PLAYTIME) == 2 && ";
	.@req$ = "isbegin_quest(5983) == 2";
	setarray .@qid,5986,5987,5988,5989,5990,5991,5992,5993,5994,5995;
	setarray .@item_id,7158,7162,7299,7266,7762,7171,6405,6498,25285,7150;
	for(.@i = 0; .@i < getarraysize(.@qid); .@i++)
		.@qid$ += "isbegin_quest(" + .@qid[.@i] + ") == -1 && ";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,.@cid$ + .@qid$ + .@req$;
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,.@cid2$ + .@qid$ + .@req$;
	for(.@i = 0; .@i < getarraysize(.@qid); .@i++)
		questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(" + .@qid[.@i] + ") && countitem(" + .@item_id[.@i] + ") >= 10";
end;
}

icecastle,27,126,3	script	#e19ms00	HIDDEN_WARP_NPC,5,5,{
	end;
	
OnTouch:
	if(isbegin_quest(5984) == 1 || isbegin_quest(17639) == 1 || isbegin_quest(16661) == 1 || isbegin_quest(17640) == 1 || isbegin_quest(17650) >= 1 && isbegin_quest(17690) != 1 || BaseLevel >= 215 && isbegin_quest(17649) == 2 && isbegin_quest(17690) == 0)
		cloaknpc("Horuru#if19msdq",false,getcharid(0));
end;
}

icecastle,27,126,3	script	Horuru#if19msdq	4_EP19_IWIN,{
	if(isbegin_quest(5984) == 1){
		mes "[Horuru]";
		mes "A Human?? Why is there a human...?";
		emotion ET_QUESTION;
		next;
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "Ah!! Are you the adventurer who came from beyond the barrier recently? Nice to meet you. Is this your first time in Isgard? I see, your first time. So, what's your name?";
		emotion ET_SURPRISE;
		next;
		cutin "",255;
		select("Who are you...?");
		cutin "ep19_iwin09.png",2;
		mes "[Horuru]";
		mes "Oh, I didn't introduce myself yet. Sorry for being rude, I was a little excited seeing strangers after coming back from a reconnaissance from afar.";
		next;
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "I'm ^0000CDHoruru^000000. That's not my full name, but humans find it hard to say, right? So, just call me Horuru.";
		mes "By the way, where have you been? It's okay to go around here alone, but don't go too far. It's dangerous.";
		next;
		cutin "",255;
		select("I was also on a reconnaissance.");
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "Reconnaissance? Ho... You must be pretty good? Are you here to report something?";
		mes "I'm on my way there, so come with me. They probably finished their meal by now and waiting at the <NAVI>[restaurant]<INFO>icas_in,34,189,0,101,0</INFO></NAVI>.";
		completequest 5984;
		setquest 17639;
		close2;
		cutin "",255;
		cloaknpc("Horuru#if19msdq",true,getcharid(0));
		end;
	}
	if(isbegin_quest(17639) == 1){
		mes "[Horuru]";
		mes "What are you doing? Are you not coming? Have you ever been at the castle's <NAVI>[restaurant]<INFO>icas_in,34,189,0,101,0</INFO></NAVI>? Everyone should be there by now.";
		close3;
	}
	if(isbegin_quest(16661) == 1){
		cutin "ep19_iwin07.png",2;
		mes "[Horuru]";
		mes "What is this? Why did you bring a Rgan here??";
		next;
		cutin "",255;
		select("I need a place to lie her down.");
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "My <NAVI>[house]<INFO>icas_in,247,116,0,101,0</INFO></NAVI> is near here, so let's go there for now. There other guests are also there too, go ahead and I'll listen to what you have to say there.";
		completequest 16661;
		setquest 17640;
		close2;
		cutin "",255;
		cloaknpc("Horuru#if19msdq",true,getcharid(0));
		end;
	}
	if(isbegin_quest(17640) == 1){
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "Hurry and move her to my <NAVI>[house]<INFO>icas_in,247,116,0,101,0</INFO></NAVI>.";
		close2;
		cutin "",255;
		cloaknpc("Horuru#if19msdq",true,getcharid(0));
		end;	
	}
	if(isbegin_quest(17650) == 1){
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "We're supposed to keep an eye on the snake's nest and search the outside for another entrance, right?";
		next;
		mes "[Horuru]";
		mes "We've already sent a scout inside the snake's nest. We need to find another entrance to get in and out of there...";
		next;
		mes "[Horuru]";
		mes "We have marked several places where we can dig through the entrance mainly from above the grounds of the snake's nest.";
		next;
		cutin "",255;
		select("That was fast.");
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "That's why we have to search. As there are not only one or two places, I asked for your help since we are lacking the manpower.";
		next;
		cutin "",255;
		select("Do I just have to search around that place?");
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "No, you have to ^0000cdstay^000000 for there.";
		next;
		cutin "",255;
		emotion ET_HUK,getcharid(3);
		select("What???");
		cutin "ep19_iwin08.png",2;
		mes "[Horuru]";
		mes "How would we know if they're going to dig a new entrance and leave?";
		cloaknpc("Lazy#if19msdq",false,getcharid(0));
		cloaknpc("Juncea#if19msdq",false,getcharid(0));
		next;
		cutin "ep19_leizi01.png",0;
		mes "[Lazy]";
		mes "Adventurer!! You're here. I heard you're on a search mission, so I came here to help you!";
		next;
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "Did you just say that you would stay in the marked entrances and monitor it? In this cold?";
		next;
		cutin "ep19_iwin06.png",1;
		mes "[Horuru]";
		mes "It's fine since we don't get cold.";
		next;
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "Every who lives here is crazy. Do you mean this isn't cold for you?";
		next;
		cutin "ep19_leizi02.png",0;
		mes "[Lazy]";
		mes "Juncea, what are you talking about when you're dressed so thin?";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "Of course, I made and installed a heating device in it.";
		npctalk "Human Juncea is so talented.","Lazy#if19msdq",bc_self;
		next;
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "What about you? Why are you wearing thick clothes and leave the front open?";
		next;
		cutin "ep19_leizi01.png",0;
		mes "[Lazy]";
		mes "Oh, this? My body temperature is high and I don't like feeling to stuffy.";
		next;
		cutin "ep19_leizi02.png",0;
		mes "[Lazy]";
		mes "Actually, it's okay for me not to wear this kind of clothes in this weather, I only wore it because I thought the people around me would feel cold when they see me.";
		next;
		cutin "ep19_leizi03.png",0;
		mes "[Lazy]";
		mes "Imagine me wearing my usual tourist outfit at this place with this weather. Wouldn't it be colder for the people around me?";
		next;
		select("That would be a problem.");
		cutin "ep19_leizi01.png",0;
		mes "[Lazy]";
		mes "Right? I wore it while enduring the frustartion because I was considerate of what the others see. I'm such a caring person.";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "Oh, okay. Just stop talking. I get tired just listening to you talk.";
		next;
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "I don't need you, why did you follow me? I won't away. I can't go throught he barrier anyway, so I'll just be in this place even if I run away. You don't have to monitor me.";
		next;
		cutin "ep19_leizi01.png",0;
		mes "[Lazy]";
		mes "What are you talking about, I came here to look around. Then, Human Juncea why are you here? Didn't you say that you'd tell me?";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "...for the sunlight.";
		next;
		cutin "ep19_leizi02.png",0;
		mes "[Lazy]";
		mes "Ah, of course. Human Juncea do need some sunlight. How long have you been in that nest?";
		npctalk "Can someone give here some sunlight?","Lazy#if19msdq",bc_self;
		next;
		cutin "ep19_iwin06.png",1;
		mes "[Horuru]";
		mes "Why did you people came here? Are you all here to help with the surveillance?";
		next;
		cutin "ep19_leizi01.png",0;
		mes "[Lazy]";
		mes "Oh, Yes. Human Juncea here made something useful to help us with the monitoring. I'll give it to you. Now, I'll let the expert explain.";
		next;
		mes "[Lazy]";
		mes "All of the people here doesn't understand science, so please explain it in a way that even a newborn will understand it.";
		npctalk "I was just born. *Cries*-","Lazy#if19msdq",bc_self;
		npctalk "...I really hate you.","Juncea#if19msdq",bc_self;
		next;
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "Remember when I said that there wasn't mana left in the Rgan's nest? It's a device that checks it the other way around.";
		next;
		mes "[Juncea]";
		mes "It only detects the wavelength of a Rgans Mana, bury it nearby and when a Rgan appear nears this device, it will catch it's wavelength and send a signal over here.";
		next;
		mes "[Juncea]";
		mes "You will see the coordinates of where it was buried, so the person waiting nearby will be able to respond quickly.";
		next;
		cutin "ep19_iwin06.png",1;
		mes "[Horuru]";
		mes "So, you want me to ^0000CDbury it where a Rgan might show up^000000?";
		next;
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "Yes. But there's a downside. It's a trivial problem, maybe? The battery of it only last a day.";
		npctalk "It takes great skill to make it endure the cold. According to Human Juncea.","Lazy#if19msdq",bc_self;
		npctalk "......","Juncea#if19msdq",bc_self;
		next;
		select("We have to replace it everyday.");
		cutin "ep19_iwin06.png",1;
		npctalk "Yes.","Juncea#if19msdq",bc_self;
		mes "[Horuru]";
		mes "That's easy. It's better than standing in the same place for 24 hours to monitor. Thank you. Thanks to this, other monitoring personnel can be dispatched somewhere else.";
		next;
		select("It looks like a clam.");
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "So that the creatures around it won't find it interesting, right? For your information, I'm not the one who named it.";
		next;
		cutin "ep19_leizi01.png",0;
		mes "[Lazy]";
		mes "I named it! ^0000CDHyper Strong Feeler^000000! Isn't it very intruitive? I named it wwith great care so that anyone who hears it will immediately understand it.";
		unittalk getcharid(3),strcharinfo(0) + " : " + "We shouldn't let Lazy give a name for stuffs like this in the future.",bc_self;
		npctalk "I agree.","Juncea#if19msdq",bc_self;
		npctalk "Just wow! Human Juncea, if you were the one who named it, it would have been something like HSF-00!!!","Lazy#if19msdq",bc_self;
		next;
		cutin "ep19_iwin06.png",1;
		mes "[Horuru]";
		mes "Thank you very much for your help. We can split up and bury them quickly. Adventurer please bury this in ^0000cd10 places^000000.";
		next;
		cutin "",255;
		select("Alright.");
		cutin "ep19_iwin06.png",1;
		mes "[Horuru]";
		mes "Then, bury this in the marked places in ^0000CDFrozen Scale Plains and the Glacier^000000. Both places are where Rgans often appear.";
		next;
		cutin "ep19_leizi01.png",0;
		mes "[Lazy]";
		mes "Goodluck!!";
		completequest 17650;
		setquest 17651;
		setquest 17652;
		setquest 17653;
		setquest 17654;
		setquest 17655;
		setquest 17656;
		setquest 17657;
		setquest 17658;
		setquest 17659;
		setquest 17660;
		getitem 1000842,10;
		close3;
	}
	for(.@i = 17651; .@i < 17661; .@i++)
		if(isbegin_quest(.@i) == 1) .@bool = true;
	if(.@bool){
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "It seems you haven't been able to bury all the Hyper Strong Feeler in ^0000CDFrozen Scale Plain^000000.";
		mes "Do this for me as a favor.";
		close3;
	}
	if(isbegin_quest(17661) == 1){
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "Adventurer, you're here. Did everything go smoothly? Thank you for your hard work. Please take a rest and let's work on it again tomorrow.";
		erasequest 17661;
		setquest 17662;
		addreputation("Ice Castle",5);
		getitem 1000608,2 + (getreputation("Ice Castle") >= 3000 ? 1 : 0);
		close3;
	}
	/*
	//Episode 20 Starting
	if(BaseLevel >= 215 && isbegin_quest(17649) == 2 && isbegin_quest(17690) == 0){
		mes "[Horuru]";
		mes "모험가, 마침 잘 오셨소.";
		next;
		mes "[Horuru]";
		mes "뱀 소굴의 입구를 찾으려 우리가 수색을 했잖소.";
		next;
		select("네, 오늘도 더듬이를 받으러 왔는데요.");
		mes "[Horuru]";
		mes "분하게도 그간 입구를 찾지 못했지 않소. 분명히 놈들이 드나드는 곳이 있는데.";
		next;
		mes "[Horuru]";
		mes "놈들은 뭔가 꾸미고 있는데, 기존의 수색지만 살펴 보다가는 성과 없이 시간만 낭비할 것 같아 수색지의 범위를 넓히기로 했소.";
		next;
		mes "[Horuru]";
		mes "그에 관한 이야기를 하기 위해 모험가를 기다리고 있었던 것이오.";
		next;
		mes "[Horuru]";
		mes "수색 인원을 새로이 편성해야 하니 어서 성 안으로 들어갑시다.";
		next;
		select("새로운 수색지는 어디인가요?");
		mes "[Horuru]";
		mes "추가된 수색지에 대한 자세한 정보 또한 안에서 자세히 이야기해줄 테니 어서 가는 것이 좋겠소. 다들 기다리고 있으니.";
		setquest 17690;
		close2;
		cutin "",255;
		cloaknpc(strnpcinfo(0),false,getcharid(0));
		end;
	}
	*/
	switch(checkquest(17662,PLAYTIME)){
		case -1:
			break;
			
		case 0:
		case 1:
			cutin "ep19_iwin06.png",2;
			mes "[Horuru]";
			mes "Adventurer, did you get some rest? Why did you come here then? You don't have to worry about the hyper strong feelers. There still a long way to go before the battery runs out. You better get some more rest.";
			close3;
			
		case 2:
			cutin "ep19_iwin06.png",2;
			mes "[Horuru]";
			mes "You're here, adventurer? I'm thinking of replacing the batteries of the hyper strong feelers before it run outs, can you help me today?";
			next;
			if(select("Sure.:I'm just passing by.") == 2){
				mes "[Horuru]";
				mes "Alright, but hurry up. Those batteries are about to run out.";
				close3;
			}
			mes "[Horuru]";
			mes "Thank you for your help. Then, please place this on the ^0000CD10 Marked Places^000000 on ^0000CDFrozen Scale Plain and Glacier^000000. Both places have possible entrances and exits for the Rgans.";
			erasequest 17662;
			setquest 17651;
			setquest 17652;
			setquest 17653;
			setquest 17654;
			setquest 17655;
			setquest 17656;
			setquest 17657;
			setquest 17658;
			setquest 17659;
			setquest 17660;
			getitem 1000842,10;
			close3;
	}

	end;

OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(5984) == 1";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16661) == 1";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17650) == 1";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17661) == 1";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17661) == 1 && checkquest(17662,PLAYTIME) == 2";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"BaseLevel >= 215 && isbegin_quest(17649) == 2 && isbegin_quest(17690) == 0";
end;
}

jor_back2,222,123,7	script	Marked Place#e19ms_1	4_POINT_WHITE,{
	.@id = atoi(replacestr(strnpcinfo(2),"e19ms_",""));
	.@qid = 17650 + .@id;
	if(isbegin_quest(.@qid) == 1){
		mes "[" + strcharinfo(0) + "]";
		mes "Is it here? Do I just bury the clam here?";
		next;
		progressbar_npc "FFFF00",1;
		if(rand(1,100) >= 60){
			mes "[" + strcharinfo(0) + "]";
			mes "It's not digging as well I thought it would. Let's do it again.";
			close;
		}
		mes "[" + strcharinfo(0) + "]";
		mes "It'll work fine here!";
		delitem 1000842,1;
		erasequest .@qid;
		questinfo_refresh;
		for(.@i = 17651; .@i < 17661; .@i++)
			if(isbegin_quest(.@i)) .@bool = true;
		if(.@bool)
			mes "Now I have to find another place to bury this.";
		else {
			mes "Now, let's return to <NAVI>[Horuru]<INFO>icecastle,27,126,0,101,0</INFO></NAVI> and report it.";
			setquest 17661;
		}
		close;
	}
	unittalk getcharid(3),strcharinfo(0) + " : This land is full of white snow and ice.",bc_self;
	end;
	
OnInit:
	.@id = atoi(replacestr(strnpcinfo(2),"e19ms_",""));
	.@qid = 17650 + .@id;
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(" + .@qid + ") == 1";
end;
}

jor_back2,49,175,7	duplicate(Marked Place#e19ms_1)	Marked Place#e19ms_2	4_POINT_WHITE
jor_back2,308,99,7	duplicate(Marked Place#e19ms_1)	Marked Place#e19ms_3	4_POINT_WHITE
jor_back2,113,235,7	duplicate(Marked Place#e19ms_1)	Marked Place#e19ms_4	4_POINT_WHITE
jor_back2,279,307,7	duplicate(Marked Place#e19ms_1)	Marked Place#e19ms_5	4_POINT_WHITE
jor_back3,152,359,7	duplicate(Marked Place#e19ms_1)	Marked Place#e19ms_6	4_POINT_WHITE
jor_back3,328,277,7	duplicate(Marked Place#e19ms_1)	Marked Place#e19ms_7	4_POINT_WHITE
jor_back3,128,138,7	duplicate(Marked Place#e19ms_1)	Marked Place#e19ms_8	4_POINT_WHITE
jor_back3,264,133,7	duplicate(Marked Place#e19ms_1)	Marked Place#e19ms_9	4_POINT_WHITE
jor_back3,46,324,7	duplicate(Marked Place#e19ms_1)	Marked Place#e19ms_10	4_POINT_WHITE

jor_back2,365,39,3	script	Pit#ep19_2_1	4_CRACK,{
	if(isbegin_quest(18135) == 1){
		if(rand(1,100) > 95){
			mes "A hole is dug through the snow.";
			next;
			if(select("Dig a hole.:Cover it well.") == 2)
				end; //TODO check possible dialogue
			progressbar "4D4DFF",2;
			mes "Woah!";
			mes "The victime was crouching in the snow pit.";
			mes "Let's take him into the Ice Castle and warm him up.";
			erasequest 18135;
			setquest 18136;
			close;
		}
	}
	mes "There are traces of digging on the snow.";
	mes "I don't see anything special around it.";
	close;
	
OnInit:
	questinfo QTYPE_CLICKME,QMARK_YELLOW,"isbegin_quest(18135) == 1";
end;
}

jor_back2,354,53,3	duplicate(Pit#ep19_2_1)	Pit#ep19_2_2	4_CRACK
jor_back2,256,197,3	duplicate(Pit#ep19_2_1)	Pit#ep19_2_3	4_CRACK
jor_back2,125,295,3	duplicate(Pit#ep19_2_1)	Pit#ep19_2_4	4_CRACK
jor_back2,195,161,3	duplicate(Pit#ep19_2_1)	Pit#ep19_2_5	4_CRACK
jor_back2,274,178,3	duplicate(Pit#ep19_2_1)	Pit#ep19_2_6	4_CRACK
jor_back2,216,336,3	duplicate(Pit#ep19_2_1)	Pit#ep19_2_7	4_CRACK
jor_back2,142,91,3	duplicate(Pit#ep19_2_1)	Pit#ep19_2_8	4_CRACK
jor_back2,127,34,3	duplicate(Pit#ep19_2_1)	Pit#ep19_2_9	4_CRACK
jor_back2,20,130,3	duplicate(Pit#ep19_2_1)	Pit#ep19_2_10	4_CRACK

jor_back3,344,296,3	script	Pit#ep19_3_1	4_CRACK,{
	if(isbegin_quest(18134) == 1){
		if(rand(1,100) > 95){
			mes "A hole is dug through the snow.";
			next;
			if(select("Dig a hole.:Cover it well.") == 2)
				end; //TODO check possible dialogue
			progressbar "4D4DFF",2;
			mes "Woah!";
			mes "The victime was crouching in the snow pit.";
			mes "Let's take him into the Ice Castle and warm him up.";
			erasequest 18134;
			setquest 18136;
			close;
		}
	}
	mes "There are traces of digging on the snow.";
	mes "I don't see anything special around it.";
	close;
	
OnInit:
	questinfo QTYPE_CLICKME,QMARK_YELLOW,"isbegin_quest(18134) == 1";
end;
}

jor_back3,320,208,3	duplicate(Pit#ep19_3_1)	Pit#ep19_3_2	4_CRACK
jor_back3,182,305,3	duplicate(Pit#ep19_3_1)	Pit#ep19_3_3	4_CRACK
jor_back3,110,269,3	duplicate(Pit#ep19_3_1)	Pit#ep19_3_4	4_CRACK
jor_back3,277,208,3	duplicate(Pit#ep19_3_1)	Pit#ep19_3_5	4_CRACK
jor_back3,197,178,3	duplicate(Pit#ep19_3_1)	Pit#ep19_3_6	4_CRACK
jor_back3,107,298,3	duplicate(Pit#ep19_3_1)	Pit#ep19_3_7	4_CRACK
jor_back3,231,68,3	duplicate(Pit#ep19_3_1)	Pit#ep19_3_8	4_CRACK
jor_back3,151,74,3	duplicate(Pit#ep19_3_1)	Pit#ep19_3_9	4_CRACK
jor_back3,39,322,3	duplicate(Pit#ep19_3_1)	Pit#ep19_3_10	4_CRACK

icecastle,25,126,5	script	Lazy#if19msdq	4_EP19_LAZY,{
	npctalk "Have you looked around the place?","",bc_self;
	end;
}

icecastle,29,125,3	script	Juncea#if19msdq	4_EP19_JUNCEA,{
	npctalk "It's cold but the air is fresh and clean.","",bc_self;
	end;
}

icas_in,34,189,5	script	Aurelie Petit#e19ms00	4_EP19_AURELIE,{
	if(isbegin_quest(17639) == 1){
		cutin "ep19_aurelie01.png",1;
		mes "[Aurelie]";
		mes "Adventurer, you're just in time. The Iwin said that you're going to attend.";
		next;
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "It seems that everyone is gathered. Sorry I'm late. I'm Horuru.";
		next;
		cutin "ep19_aurelie01.png",1;
		mes "[Aurelie]";
		mes "Now that we're all here, let's start the meeting. It's bad news but there is indeed a lot of suspicious activities from the cult.";
		next;
		cutin "ep19_leon01.png",0;
		mes "[Leon]";
		mes "Is this a sign that the ^0000CDJormungandr cult have fully revived^000000? Up until they were just hiding.";
		next;
		cutin "ep19_vellgunde02.png",1;
		mes "[Vellgunde]";
		mes "Maybe the ^0000CDIllusion^000000 that came here has something to do with it.";
		next;
		cutin "ep19_aurelie01.png",1;
		mes "[Aurelie]";
		mes "If the cult have truly revived, the continent will face another crisis. Unlike back then, there are not that many forces that we can rally.";
		mes "It's also not easy to cross the barrier to enter.";
		next;
		cutin "ep18_miriam_01.png",1;
		mes "[Miriam]";
		mes "Not only the cult is the problem, the Illusion also has the stolen ^0000CDYmir Heart Fragment^000000.";
		next;
		cutin "ep19_voglinde01.png",1;
		mes "[Voglinde]";
		mes "However, it's not easy to infiltrate their base. We need to figure out a way...";
		next;
		cutin "ep19_leizi01.png",2;
		mes "[Lazy]";
		mes "We'll have to check the structure and power of their home base to figure anything out. They also look different, it limits my movement, I can only look at them from afar.";
		next;
		cutin "ep19_voglinde01.png",1;
		mes "[Voglinde]";
		mes "Does the Iwins know anything? They've lived here with the Rgans for a very long time.";
		next;
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "We've been looking for a way for a long time, but surprisingly there's only one entry point. There might be a hidden entrance but we haven't found anything yet.";
		next;
		cutin "ep19_leizi01.png",2;
		mes "[Lazy]";
		mes "What about their powers?";
		next;
		cutin "ep19_aurelie01.png",1;
		mes "[Aurelie]";
		mes "Did you know that they can use magical powers? Their ^0000CDmana^000000 has trouble countless warrios in the past. I tried to figure out their magic, but it's not easy as it seems, don't you think?";
		next;
		cutin "ep19_lehar01.png",0;
		mes "[Lehar]";
		mes "How about we dig a hole and make a surprise attack...?";
		next;
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "It's not that simple. Their base is underground, we don't know the depth of it.";
		next;
		cutin "ep19_leizi03.png",2;
		mes "[Lazy]";
		mes "Wow... There is really no information, there's no easy way. Either way, I have to go in there and check it for myself.";
		next;
		select("By the way, on the way here...");
		cutin "ep19_leizi01.png",2;
		mes "[Lazy]";
		mes "You got it on hand, what is it?";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "A friend that I found during a patrol had something interesting. Do you want to see it?.";
		next;
		specialeffect2 EF_FOOD03;
		transform 21529,180000,SC_MONSTER_TRANSFORM;
		npctalk "Oh, It's a Rgan...!","Horuru#e19ms00",bc_self;
		sleep2 1500;
		cutin "ep19_iwin07.png",1;
		mes "[Horuru]";
		mes "So that's what special about it!!";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "Wait! I'm not really a Rgan, I just transformed into one! Everyone, put down your weapons.";
		next;
		npctalk "Transformed into a Rgan...??","Aurelie Petit#e19ms00",bc_self;
		npctalk "How?","Lazy#e19ms00",bc_self;
		npctalk "Undeniably, you look exactly like one!!","Leon Petit#e19ms00",bc_self;
		specialeffect2 EF_LIGHTSPHERE;
		if(getstatus(SC_MONSTER_TRANSFORM)) sc_end SC_MONSTER_TRANSFORM;
		mes "[" + strcharinfo(0) + "]";
		mes "Lunch is a human that was held in the Rgan's base, and he escaped by making a scroll that transformed him into a Rgan.";
		next;
		select("I found him and brought this back with me.");
		cutin "ep19_leizi01.png",2;
		npctalk "Ho... a Rgan Transformation Scroll?","Vellgunde#e19ms00",bc_self;
		npctalk "How does it work?","Voglinde#e19ms00",bc_self;
		mes "[Lazy]";
		mes "Wow~~ that's really cool~~ I suddenly had a good idea.";
		next;
		cutin "ep19_leizi03.png",2;
		mes "[Lazy]";
		mes "Let's use that.";
		next;
		cutin "ep19_leizi02.png",2;
		unittalk getcharid(3),"What...?",bc_self;
		mes "[Lazy]";
		mes "Don't tell me you're just amazed?";
		mes "If we use that we might be able to do it. Use it to infiltrate the Rgans base.";
		next;
		npctalk "......","Lazy#e19ms00",bc_self;
		npctalk "......","Voglinde#e19ms00",bc_self;
		npctalk "......","Lehar#e19ms00",bc_self;
		npctalk "......","Vellgunde#e19ms00",bc_self;
		npctalk "......","Miriam#e19ms00",bc_self;
		npctalk "......","Horuru#e19ms00",bc_self;
		npctalk "......","Leon Petit#e19ms00",bc_self;
		npctalk "......","Aurelie Petit#e19ms00",bc_self;
		sleep2 1500;
		emotion ET_SURPRISE,getnpcid(0,"Lazy#e19ms00");
		emotion ET_SURPRISE,getnpcid(0,"Voglinde#e19ms00");
		emotion ET_SURPRISE,getnpcid(0,"Lehar#e19ms00");
		emotion ET_SURPRISE,getnpcid(0,"Vellgunde#e19ms00");
		emotion ET_SURPRISE,getnpcid(0,"Miriam#e19ms00");
		emotion ET_SURPRISE,getnpcid(0,"Horuru#e19ms00");
		emotion ET_SURPRISE,getnpcid(0,"Leon Petit#e19ms00");
		emotion ET_SURPRISE,getnpcid(0,"Aurelie Petit#e19ms00");
		cutin "ep18_maram_01.png",1;
		mes "[Maram]";
		mes "I know???? We can use that to ^0000CDspy on them or search^000000 their base!!";
		next;
		cutin "ep19_vellgunde02.png",1;
		mes "[Vellgunde]";
		mes "Is there any danger of you being caught? Rgan has mana, they might be able to use it sense each other.";
		next;
		cutin "ep19_leizi01.png",2;
		mes "[Lazy]";
		mes "If they did, he wouldn't be able to escape.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "The transformation will be released after some time, but we can use it continuously.";
		next;
		cutin "ep19_aurelie01.png",1;
		mes "[Aurelie]";
		mes "Well... if it's a transformation that even the Rgan won't be able to recognize...";
		next;
		cutin "ep19_leizi01.png",2;
		mes "[Lazy]";
		mes "That's Great. Give me one.";
		next;
		select("Lazy? What are you going to do?");
		cutin "ep19_leizi02.png",2;
		mes "[Lazy]";
		mes "Of course. I need it to get some information. Does anyone know what's going in there? No. Isn't that why we need some information?";
		next;
		mes "[Lazy]";
		mes "A whole new world and race. Don't you want to find out the structure and their power?";
		mes "Am I the only one who's curious?";
		next;
		select("Why would you do that?");
		cutin "ep19_leizi03.png",2;
		mes "[Lazy]";
		mes "Can't I do it? Why do you talk like you're unhappy with it?";
		mes "I will only tell you this because you seem to forgot, isn't it my job to gather information? Is anyone here better than me at gathering information??";
		next;
		cutin "ep19_leizi02.png",2;
		mes "[Lazy]";
		mes "A person who digs up information so well that it ended up as his job??";
		npctalk "I know a little something about your nephew's past over there. That, what... the darkness... is it something like that...?","Lazy#e19ms00",bc_self;
		npctalk "Ah!!!!","Lehar#e19ms00",bc_self;
		next;
		cutin "ep19_leizi01.png",2;
		mes "[Lazy]";
		mes "There's no one. No one. So I'll go.";
		mes "It's not because I just wanted to use this scroll.";
		next;
		cutin "ep18_miriam_01.png",1;
		mes "[Miriam]";
		mes "But it would be dangerous.";
		next;
		cutin "ep19_leizi01.png",2;
		mes "[Lazy]";
		mes "Oh, it's okay, it's my personal problem. Also the adventurer will be there for my self-defense.";
		mes "And do you think I have felt safe in my daily life? Everyday is always a threat for me.";
		next;
		select("Me?");
		cutin "ep19_voglinde01.png",1;
		mes "[Voglinde]";
		mes "Even so, If you're going to infiltrate the base. I can't just send two people.";
		next;
		cutin "ep19_vellgunde02.png",1;
		mes "[Vellgunde]";
		mes "If you're that worried. Just add up two or three more. If you go with a huge number, there's a high probability that you'll be found out.";
		next;
		cutin "ep18_miriam_01.png",1;
		mes "[Miriam]";
		mes "Then, I'll go.";
		next;
		cutin "ep19_lehar04.png",0;
		mes "[Lehar]";
		mes "I'm going too.";
		next;
		cutin "ep19_iwin06.png",1;
		mes "[Horuru]";
		mes "I'll also go. This Iwin will take the lead.";
		next;
		cutin "ep19_leizi01.png",2;
		mes "[Lazy]";
		mes "Then, the three of you can be in the same group. The adventurer will come with me.";
		next;
		select("Why me??");
		cutin "ep19_leizi02.png",2;
		mes "[Lazy]";
		mes "Hey, Isn't the transformation scroll on your hand is a sign that you're coming with me?";
		next;
		cutin "ep19_vellgunde02.png",1;
		mes "[Vellgunde]";
		mes "Rgans have a certain amount of mana in their bodies, so it's better for everyone to bring some mana cores on their bodies.";
		next;
		cutin "ep19_leizi01.png",2;
		mes "[Lazy]";
		mes "It's good to be thoroughly prepared. Please give me a lot";
		next;
		cutin "ep19_leon01.png",0;
		mes "[Leon]";
		mes "Don't strain yourself. Remember, it's the <NAVI>[base]<INFO>jor_back3,98,318,0,101,0</INFO></NAVI> of the Rgans.";
		completequest 17639;
		setquest 17619;
		close3;
	}
	if(isbegin_quest(17619) == 1){
		cutin "ep19_aurelie01.png",0;
		mes "[Aurelie]";
		mes "Didn't you say that you're going to check out the <NAVI>[Rgans]<INFO>jor_back3,98,318,0,101,0</INFO></NAVI>? Be careful.";
		close3;
	}
	if(isbegin_quest(17636) == 1){
		cutin "ep19_aurelie01.png",1;
		mes "[Aurelie]";
		mes "Ah! You're here. Did everything go well?";
		npctalk "Did you get something?","Leon Petit#e19ms00",bc_self;
		next;
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "It won't be easy. I looked at the structure inside, and there was only one entrance, one exit, and it's very complicated inside.";
		mes "A simultaneous raid would be difficult..";
		next;
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "It has its own defenses, it seems impossible to a dig a new entrance without them knowing.";
		next;
		cutin "ep19_lehar01.png",0;
		mes "[Lehar]";
		mes "They have a lof of numbers, ranking from the highest to lowest, they all possess mana, I haven't determined their power.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "Even the lower-grade are stronger than average humans thanks to their mana.";
		next;
		cutin "ep19_lehar01.png",0;
		mes "[Lehar]";
		mes "Their leader is ^0000CDLasgand^000000. A survivor from the previous battle with Jormungandr.";
		next;
		mes "[Lehar]";
		mes "They formed a huge following around Lasgand. He's very influential. Like a religious leader, he's in control of everything.";
		next;
		cutin "ep19_leon01.png",1;
		mes "[Leon]";
		mes "If we catch Lasgand, the rest will be easy to handle.";
		mes "But catching him won't be easy.";
		next;
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "We'll need a lot of mana cores. If you want to catch Lasgand, we'll need a way to get in and out, but neither of it seems easy at this point.";
		next;
		cutin "ep19_lehar01.png",0;
		mes "[Lehar]";
		mes "Luckily, Lazy can transform into a superior Rgan, so it might be possible to do something with Rasgan...";
		next;
		cutin "ep19_leizi03.png",2;
		mes "[Lazy]";
		mes "Oh, It's not that I haven't thought of that. Superior Rgans usually lives for hundreds years.";
		next;
		cutin "ep19_leon01.png",1;
		mes "[Leon]";
		mes "Even though he is a superior-grade, it would be difficult for a newborn Rgan to be the key to handle Lasgand.";
		next;
		cutin "ep19_leizi02.png",2;
		mes "[Lazy]";
		mes "It's hard, but it's not impossible. I just need a little time. I'll do my best. They also show interest in me.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "At present, only a handful of has access to Lasgand, a few close aides and Bagot, the one escaped from Rachel.";
		next;
		cutin "ep19_leon01.png",1;
		mes "[Leon]";
		mes "How can an Illusion, an outsider has access with their religious leader?";
		next;
		cutin "ep19_leizi03.png",2;
		mes "[Lazy]";
		mes "They were in a symbiotic relationship with Rgans. In a way that they actively cooperate with what Lasgand wants and return, they'll support their research.";
		next;
		cutin "ep19_aurelie01.png",1;
		mes "[Aurelie]";
		mes "How many Illusion are there?";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "Bagot, and the other who came before him, Juncea. There's only two of them. And several Heart Hunters.";
		next;
		cutin "ep19_aurelie03.png",1;
		mes "[Aurelie]";
		mes "He was the one who were in the airship when he interfered with the barrier.";
		next;
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "What about the Heart of Ymir? Have you checked it?";
		next;
		cutin "ep19_leizi01.png",2;
		mes "[Lazy]";
		mes "I haven't seen it, but I'm sure Bagot has it. He most likely got Lasgand's interest by using the heart for research.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "Whatever the research was, I think Lasgand liked it enough to give all the resources of Bagot from Juncea, who have arrived first and researched.";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "Juncea's guess is that the research amplifies the remaining mana in the place or a way to absorb the mana of the heart.";
		next;
		cutin "ep19_leizi01.png",2;
		mes "[Lazy]";
		mes "Lasgand really liked the documents that Bagot brought.";
		mes "He said that their future depends on Bagot, and it won't be long before they return to their glory days.";
		next;
		cutin "ep19_aurelie01.png",1;
		mes "[Aurelie]";
		mes "What was it?";
		next;
		cutin "ep19_leizi02.png",2;
		mes "[Lazy]";
		mes "To what was it. I really wanted to know, but unfortunately, Lasgand didn't show me. The newborn can only cry.";
		next;
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "Whatever it is, we are in a situation where we can't simply stand by.";
		next;
		cutin "ep19_aurelie01.png",1;
		mes "[Aurelie]";
		mes "There's a chance that they'll run away again after dealing with the cult.";
		next;
		cutin "ep19_leon01.png",1;
		mes "[Leon]";
		mes "You are right. I want to secure as quickly as possible.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "I don't think they'll come out easily. They seemed to be wary after hearing that an outside had appeared.";
		next;
		cutin "ep19_leizi01.png",2;
		mes "[Lazy]";
		mes "The boundaries are high. But we can break those boundaries? Juncea was very friendly to us, don't you think? We can use her to approach them.";
		mes "Let's stick with her and learn more about her research.";
		next;
		cutin "ep19_lehar01.png",0;
		mes "[Lehar]";
		mes "We'll go back and look for more vulnerabilities in the facility. We also know the size of the people who have been captured.";
		next;
		cutin "ep19_aurelie01.png",1;
		mes "[Aurelie]";
		mes "Are those captured people in one place?";
		next;
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "They're all over the place. Humans that are captured are mainly used for experiments or forced labor in the lower level.";
		npctalk "We got out of there.","Lehar#e19ms00",bc_self;
		next;
		cutin "ep19_lehar01.png",0;
		mes "[Lehar]";
		mes "To prevent the escape of the Illusion and the abduction of innocent humans, we'd better deal with their airship.";
		next;
		cutin "ep19_aurelie01.png",1;
		mes "[Aurelie]";
		mes "Then, you'll have go back to the snake's nest. Be careful.";
		next;
		cutin "ep19_leizi01.png",2;
		mes "[Lazy]";
		mes "Adventurer! wait, I'll give this to you before I go.";
		next;
		select("What's this");
		mes "[Lazy]";
		mes "The airship, it's currently stuck. Since it was made by Varmundt, it won't be easy to use, right?";
		next;
		select("I... guess so?");
		mes "[Lazy]";
		mes "That's why I wrote something down for you. You remember the airship navigator?";
		next;
		select("I know. It's Ginger.");
		cutin "ep19_leizi02.png",2;
		mes "[Lazy]";
		mes "Yes, her. I asked her some question. I wrote down what I've heard from her, use it for our work. Every information payoff. Right?";
		next;
		mes "[Lazy]";
		mes "I was planning to put it in a nice new paper with beautiful handwriting and write it like a report, but I was too busy. Anyway, goodluck!";
		npctalk "That's the only piece of paper I have, so I wrote it down there, you can read it with no problem.","Lazy#e19ms00",bc_self;
		next;
		select("Aren't you coming with me Lazy?");
		cutin "ep19_leizi01.png",2;
		mes "[Lazy]";
		mes "Of course I will go too. But I can't control the airship. To destroy it with my own hands? I can't do it.";
		next;
		mes "[Lazy]";
		mes "Anyway, let's hurry. Oh, you'd better speak more fluently this time. Apparently Rgans are already good at talking right after they're born. Let's go!";
		completequest 17636;
		setquest 17637;
		setquest 17638;
		getitem 1000608,50;
		addreputation("Ice Castle",50);
		close3;
	}
	if(isbegin_quest(17638) == 1){
		cutin "ep19_aurelie01.png",0;
		mes "[Aurelie]";
		mes "To go back in the <NAVI>[snake's nest]<INFO>jor_nest,127,207,0,101,0</INFO></NAVI> and get the Illusion's favor to operarate... it won't be easy.";
		close3;
	}
	if(isbegin_quest(17649) == 1){
		cutin "ep19_aurelie01.png",1;
		mes "[Aurelie]";
		mes "It's heartbreaking to come back empty-handed.";
		next;
		cutin "ep19_leon01.png",1;
		mes "[Leon]";
		mes "There must be a separate entrance somewhere because they can't really just dug that place.";
		next;
		cutin "ep19_voglinde01.png",2;
		mes "[Voglinde]";
		mes "We can either make a new entrance or they already have it made.";
		next;
		select("We have to search for it.");
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "We'll take care of that. Even if that amount of people leave one by one, they will stand out. We need the adventurers help.";
		next;
		cutin "ep19_leon01.png",1;
		mes "[Leon]";
		mes "Even if they go back and forth unnoticed, they can't cross the barrier unless they use the airship. They can't get out of here.";
		next;
		cutin "ep19_lehar01.png",0;
		mes "[Lehar]";
		mes "I don't think they can use that. Because we are watching it.";
		next;
		cutin "ep19_vellgunde02.png",1;
		mes "[Vellgunde]";
		mes "She is also coming.";
		next;
		select("Who?");
		cutin "ep19_juncea04.png",2;
		cloaknpc("Juncea#e19ms00",false,getcharid(0));
		mes "[Juncea]";
		mes "Why are there only cold places here?";
		next;
		cutin "ep19_vellgunde02.png",1;
		mes "[Vellgunde]";
		mes "You're late.";
		next;
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "I'm a little late because I was cleaning up. But why do I have to share this with others?";
		npctalk "Juncea human!","Lazy#e19ms00",bc_self;
		npctalk "You!!! Don't stop calling me like that!!","Juncea#e19ms00",bc_self;
		next;
		cutin "ep19_vellgunde02.png",1;
		mes "[Vellgunde]";
		mes "Didn't you promised to cooperate with us? or should I take you up there?";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "...Okay. A promise is a promise.";
		next;
		cutin "ep19_aurelie01.png",1;
		mes "[Aurelie]";
		mes "You're the human who was betrayed while working for the Rgans? Have you been into the Rgan's nest? Do you have anything for us?";
		npctalk "I'm not working for them!","Juncea#e19ms00",bc_self;
		npctalk "Ho- Didn't you enjoy your time there? You're expression is so bright!","Lazy#e19ms00",bc_self;
		next;
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "Most of my research materials in my laboratory were taken by Bagot, so there little to recover.";
		next;
		cutin "ep19_aurelie01.png",1;
		mes "[Aurelie]";
		mes "By little, do you mean there are remains? You can trace it back and thell us how far his research has gone.";
		next;
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "Haven't you found out? What do you even want me to find out from this mess?";
		next;
		cutin "ep19_leizi01.png",2;
		mes "[Lazy]";
		mes "Juncea is a talented human who can~ do it~ Don't you want get hired by Rekenber?";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "......I'll find out. But, there's nothing I can tell you right now.";
		mes "There's almost no mana left in there. Looks like they absorbed the rest.";
		next;
		cutin "ep19_aurelie01.png",1;
		mes "[Aurelie]";
		mes "...And there was nothign special except the modified Rgans. Unlike the chimera that Bagot that the adventurer previously dealt with.";
		next;
		cutin "ep19_leon01.png",0;
		mes "[Leon]";
		mes "I had a feeling that they weren't going all out either. It felt like they were just diverting our attention to buy time. Did they needed time to close the door?";
		next;
		cutin "ep19_aurelie01.png",1;
		mes "[Aurelie]";
		mes "Or they could be gathering mana for the resurrection of Jormungandr behind that door.";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "Well. If they are planning to do that, they wouldn't have squeezed the remaining mana. Since that place is the main body.";
		mes "More than that, I'm more concerned about injecting mana to the Superior Rgans. They didn't even let me test on one. Sigh...";
		next;
		cutin "ep19_aurelie01.png",1;
		mes "[Aurelie]";
		mes "Whatever it is, if we don't find a way to get in there, we won't get an answer...";
		next;
		mes "[Aurelie]";
		mes "We need to find a way to get into the their hiding place, and at the same time, we need to be prepared for whatever may happen.";
		next;
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "We too will immediately set off on a mission to find another entrance to their hiding place.";
		next;
		cutin "ep19_aurelie01.png",1;
		mes "[Aurelie]";
		mes "Let's wrap this conversation for today. Leon and I need to check the barrier to make sure it didn't weakened. Please rest until we have more news.";
		completequest 17649;
		questinfo_refresh;
		getitem 1000608,50;
		close3;
	}
	if(isbegin_quest(17649) == 2){
		cutin "ep19_aurelie01.png",1;
		mes "[Aurelie]";
		mes "I thought we could finally annihilate them, my excitement was ruined.";
		close3;
	}
	cutin "ep19_aurelie01.png",1;
	mes "[Aurelie]";
	mes "Are you here to eat? Today's sturdy ice omelettes is delicious.";
	close3;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17639) == 1";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17636) == 1";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17649) == 1";
end;
}

icas_in,32,189,5	script	Leon Petit#e19ms00	4_EP19_LEON,{
	if(isbegin_quest(17639) == 1){
		cutin "ep19_leon01.png",0;
		mes "[Leon]";
		mes "I'm sure there was nothing special about the reconnaissance, right? Hm? Am I wrong?";
		close3;
	}
	if(isbegin_quest(17619) == 1){
		cutin "ep19_leon01.png",0;
		mes "[Leon]";
		mes "A transformation scroll, that sounds fun. I personally don't want to do it though. Be careful.";
		close3;
	}
	if(isbegin_quest(17636) == 1){
		cutin "ep19_leon01.png",0;
		mes "[Leon]";
		mes "What did you see there? Come, let's talk.";
		close3;
	}
	if(isbegin_quest(17638) == 1){
		cutin "ep19_leon01.png",0;
		mes "[Leon]";
		mes "Hmm... Block the escape route and induce a internal division. This would be interesting.";
		close3;
	}
	if(isbegin_quest(17641) == 2){
		cutin "ep19_leon01.png",0;
		mes "[Leon]";
		mes "I don't think we'll work out a plan like this... Now, let's use everyone's intelligence to devise one.";
		close3;
	}
	if(isbegin_quest(17649) == 2){
		cutin "ep19_leon01.png",0;
		mes "[Leon]";
		mes "I need to work on the barrier a bit more.";
		close3;
	}
	cutin "ep19_leon01.png",0;
	mes "[Leon]";
	mes "Why don't you stay here? Is it uncomfortable? Well, since it's not your home it must be uncomfortable.";
	close3;
}

icas_in,34,183,3	script	Lazy#e19ms00	4_EP19_LAZY,{
	if(isbegin_quest(17639) == 1){
		cutin "ep19_leizi01.png",0;
		mes "[Lazy]";
		mes "Adventurer! How was your reconnaissance? What is it? Did you found anything?";
		close3;
	}
	if(isbegin_quest(17619) == 1){
		cutin "ep19_leizi01.png",0;
		mes "[Lazy]";
		mes "When else would you have a chance to turn into a Rgan? Let's go, adventurer!";
		close3;
	}
	if(isbegin_quest(17636) == 1){
		cutin "ep19_leizi01.png",0;
		mes "[Lazy]";
		mes "Now, let everyone know about our achievement!";
		close3;
	}
	if(isbegin_quest(17638) == 1){
		cutin "ep19_leizi01.png",0;
		mes "[Lazy]";
		mes "This time, I'll have to take a closer look.";
		close3;
	}
	if(isbegin_quest(17649) == 1){
		cutin "ep19_leizi01.png",0;
		mes "[Lazy]";
		mes "Phew- that was hard. Adventurer, we have to come up with a new plan, okay?";
		close3;
	}
	if(isbegin_quest(17649) == 2){
		cutin "ep19_leizi01.png",0;
		mes "[Lazy]";
		mes "Adventurer! Why don't you take a look around with me? We can even build a snowman!";
		close3;
	}
	cutin "ep19_leizi01.png",0;
	mes "[Lazy]";
	mes "The food here, what do you think about it?";
	close3;
}

icas_in,32,184,1	script	Horuru#e19ms00	4_EP19_IWIN,{
	if(isbegin_quest(17639) == 1){
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "Didn't you say that you're here to report about the reconnaissance?";
		close3;
	}
	if(isbegin_quest(17619) == 1){
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "I can't believe that I'll be able to go into the Rgans nest, but it's a fact. This is the first time in my life that this has happened. Let's go.";
		close3;
	}
	if(isbegin_quest(17636) == 1){
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "I'd like you to share the work we've done in the Rgans Nest.";
		close3;
	}
	if(isbegin_quest(17649) == 1){
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "It's a bitter pill to swallow, but we should discuss countermeasures.";
		close3;
	}
	if(isbegin_quest(17649) == 2 && isbegin_quest(17650) == 0){
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "Adventurer, can you spare me a minute?";
		next;
		select("What's going on?");
		mes "[Horuru]";
		mes "I need to ^0000CDkeep an eye^000000 out on the snake's nest in case they make another entrance.";
		next;
		select("Yes you should.");
		mes "[Horuru]";
		mes "I need your help for that.";
		next;
		if(select("Sure.:I'm busy right now.") == 2){
			mes "[Horuru]";
			mes "Well, you'll be busy for a while because you have a lot of things to deal with right now. I understand. We'll do the search first, so please help us later.";
			close3;
		}
		mes "[Horuru]";
		mes "Thank you for your willingness to help. Then instead of wasting time here, let's go <NAVI>[outside]<INFO>icecastle,27,126,0,101,0</INFO></NAVI> and discuss our search strategy.";
		setquest 17650;
		close3;
	}
	if(isbegin_quest(17650) == 1){
			cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "Thank you for helping. Instead of wasting time here, let's go <NAVI>[outside]<INFO>icecastle,27,126,0,101,0</INFO></NAVI> and discuss our search strategy.";
		close3;
	}
	cutin "ep19_iwin06.png",2;
	mes "[Horuru]";
	mes "I prepared this for the human guests, do you like it?";
	close3;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17649) == 2 && isbegin_quest(17650) == 0";
end;
}

icas_in,32,182,1	script	Voglinde#e19ms00	4_EP19_VOGLINDE,{
	if(isbegin_quest(17639) == 1){
		cutin "ep19_voglinde01.png",0;
		mes "[Voglinde]";
		mes "Did the reconnaissance went well? Aurelie is very curious.";
		close3;
	}
	if(isbegin_quest(17619) == 1){
		cutin "ep19_voglinde01.png",0;
		mes "[Voglinde]";
		mes "Are you prepared? Would you like some more mana cores?";
		close3;
	}
	if(isbegin_quest(17636) == 1){
		cutin "ep19_voglinde01.png",0;
		mes "[Voglinde]";
		mes "I'm really curious about what you're going to tell me, I'm sure everyone is the same. Let's get started.";
		close;
	}
	if(isbegin_quest(17638) == 1){
		cutin "ep19_voglinde01.png",0;
		mes "[Voglinde]";
		mes "I'll have to go there sometime.";
		close;
	}
	if(isbegin_quest(17649) == 1){
		cutin "ep19_voglinde01.png",0;
		mes "[Voglinde]";
		mes "Well... there must be something in your mind. Let's discuss countermeasures.";
		close3;
	}
	if(isbegin_quest(17649) == 2){
		cutin "ep19_voglinde01.png",0;
		mes "[Voglinde]";
		mes "Do we really have no choice but to improve the reconnaissance unit for the future?";
		close3;
	}
	cutin "ep19_voglinde01.png",0;
	mes "[Voglinde]";
	mes "Hmm... Should I heat it up more?";
	close3;
}
	
icas_in,30,183,1	script	Vellgunde#e19ms00	4_EP19_VELLGUNDE,{
	if(isbegin_quest(17639) == 1){
		cutin "ep19_vellgunde01.png",0;
		mes "[Vellgunde]";
		mes "You went on a reconnaissance? Have you picked up on anything?";
		close3;
	}
	if(isbegin_quest(17619) == 1){
		cutin "ep19_vellgunde01.png",0;
		mes "[Vellgunde]";
		mes "That scroll, I can very it's stability by researching it closely, but I can't do it right now because I'm also busy. Use it carefully.";
		close3;
	}
	if(isbegin_quest(17636) == 1){
		cutin "ep19_vellgunde01.png",0;
		mes "[Vellgunde]";
		mes "Did you capture the Illusions? I have a lot to ask.";
		close3;
	}
	if(isbegin_quest(17638) == 1){
		cutin "ep19_vellgunde01.png",0;
		mes "[Vellgunde]";
		mes "I want to see that place too. I'll go there soon, please do your best until then.";
		close3;
	}
	if(isbegin_quest(17649) == 1){
		cutin "ep19_vellgunde01.png",0;
		mes "[Vellgunde]";
		mes "How do we pierce that place, have you thought about it? What do other people think?";
		close3;
	}
	if(isbegin_quest(17649) == 2){
		cutin "ep19_vellgunde01.png",0;
		mes "[Vellgunde]";
		mes "It will take some time for Juncea to take a look into it. Don't you think so?";
		close3;
	}
	cutin "ep19_vellgunde01.png",0;
	mes "[Vellgunde]";
	mes "It's cold, can't we make a fire here? What do you think?";
	close3;
}
	
icas_in,30,185,7	script	Miriam#e19ms00	4_EP18_MIRIAM,{
	if(isbegin_quest(17639) == 1){
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "It looks like your reconnaissance went well. I'm here to do a reconnaissance report too.";
		close3;
	}
	if(isbegin_quest(17619) == 1){
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "Let's go to the Rgans base. The Illusions will be there too.";
		close;
	}
	if(isbegin_quest(17636) == 1){
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "I took a good look at at the lower level. Adventurer, please handle the explanation for the upper level.";
		close3;
	}
	if(isbegin_quest(17638) == 1){
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "Let's take a closer look this time";
		close3;
	}
	if(isbegin_quest(17649) == 1){
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "Is there anything we can do to pierce that place?";
		close3;
	}
	if(isbegin_quest(17649) == 2){
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "I was constantly observing the Rgans residence, but I don't have much results.";
		close3;
	}
	cutin "ep18_miriam_01.png",0;
	mes "[Miriam]";
	mes "Ever since I was little I've heard stories that witches lives in the north, but there's no such thing. Right?";
	close3;
}

icas_in,29,187,7	script	Maram#e19ms00	4_EP18_MARAM,{
	if(isbegin_quest(17639) == 1){
		cutin "ep18_maram_01.png",1;
		mes "[Maram]";
		mes "Did you just come back from the reconnaissance? Have you seen anything?";
		close3;
	}
	if(isbegin_quest(17619) == 1){
		cutin "ep18_maram_01.png",1;
		mes "[Maram]";
		mes "I also want to accompany you, but I have something else to do. The scroll, I want to try it...";
		close3;
	}
	if(isbegin_quest(17636) == 1){
		cutin "ep18_maram_01.png",1;
		mes "[Maram]";
		mes "How did it go? I'm glad you came back safely.";
		close3;
	}
	if(isbegin_quest(17638) == 1){
		cutin "ep18_maram_01.png",1;
		mes "[Maram]";
		mes "I will work hard here too.";
		close3;
	}
	if(isbegin_quest(17649) > 0){
		cutin "ep18_maram_01.png",1;
		mes "[Maram]";
		mes "I had no idea things would go this bad. I already thought this wouldn't be easy, but...";
		close3;
	}
	cutin "ep18_maram_01.png",1;
	mes "[Maram]";
	mes "It's my first time here, and it's amazing. I can't believe that there's a place like this in the north...";
	close3;
}
	
icas_in,30,188,5	script	Lehar#e19ms00	4_EP19_LEHAR,{
	if(isbegin_quest(17639) == 1){
		cutin "ep19_lehar01.png",0;
		mes "[Lehar]";
		mes "Did you see anything intersted while on reconnaissance? Like, a walking fish or maybe a flying black snake!";
		close3;
	}
	if(isbegin_quest(17619) == 1){
		cutin "ep19_lehar01.png",0;
		mes "[Lehar]";
		mes "Just thinking about of using this scrolls makes me both nervous and excited. This is my first time doing such a thing!";
		close3;
	}
	if(isbegin_quest(17636) == 1){
		cutin "ep19_lehar01.png",0;
		mes "[Lehar]";
		mes "I don't have much to report, but... Adventurer, please tell them all of it...";
		close3;
	}
	if(isbegin_quest(17638) == 1){
		cutin "ep19_lehar01.png",0;
		mes "[Lehar]";
		mes "I'm going to do my best this time!";
		close3;
	}
	if(isbegin_quest(17649) == 1){
		cutin "ep19_lehar01.png",0;
		mes "[Lehar]";
		mes "What should we do now";
		close3;
	}
	if(isbegin_quest(17649) == 2){
		cutin "ep19_lehar01.png",0;
		mes "[Lehar]";
		mes "I will also join the reconnaissance!";
		close3;
	}
	cutin "ep19_lehar01.png",0;
	mes "[Lehar]";
	mes "It's cold, but it doesn't feel cold! How can I explain this?";
	close3;
}

icas_in,35,180,1	script	Juncea#e19ms00	4_EP19_JUNCEA,{
	mes "[Juncea]";
	mes "I'm a little tired.";
	close;
}

jor_back3,99,318,0	script	#e19ms11	HIDDEN_WARP_NPC,5,5,{
	end;
	
OnTouch:
	if(isbegin_quest(17619) == 1){
		cloaknpc("Horuru#e19ms12",true,getcharid(0));
		cloaknpc("Lazy#e19ms12",true,getcharid(0));
		cloaknpc("Lehar#e19ms12",true,getcharid(0));
		cloaknpc("Miriam#e19ms12",true,getcharid(0));
		cloaknpc("Horuru#e19ms11",false,getcharid(0));
		cloaknpc("Lazy#e19ms11",false,getcharid(0));
		cloaknpc("Lehar#e19ms11",false,getcharid(0));
		cloaknpc("Miriam#e19ms11",false,getcharid(0));
	}
end;
}

function	script	191_scroll_check	{
	return getstatus(SC_MONSTER_TRANSFORM,1) == 21530;
}

jor_back3,99,318,3	script	Horuru#e19ms11	4_EP19_IWIN,{
	if(isbegin_quest(17619) == 1){
		if(!191_scroll_check()){
			cutin "ep19_iwin06.png",2;
			mes "[Horuru]";
			mes "You'd better transform yourself into a Rgan using the scroll.";
			close3;
		}
		mes "[Horuru]";
		mes "There's a Rgan scout units in front of us, so it would be better to transform here.";
		next;
		specialeffect EF_FOOD02,AREA,"Horuru#e19ms11";
		cloaknpc("Horuru#e19ms11",true,getcharid(0));
		cloaknpc("Horuru#e19ms12",false,getcharid(0));
		sleep2 1000;
		specialeffect EF_FOOD02,AREA,"Lehar#e19ms11";
		cloaknpc("Lehar#e19ms11",true,getcharid(0));
		cloaknpc("Lehar#e19ms12",false,getcharid(0));
		sleep2 1000;
		specialeffect EF_FOOD02,AREA,"Miriam#e19ms11";
		cloaknpc("Miriam#e19ms11",true,getcharid(0));
		cloaknpc("Miriam#e19ms12",false,getcharid(0));
		cutin "",255;
		mes "[Miriam]";
		mes "Ho... So this is a Rgans body, It'll take practice to walk like a Rgan.";
		next;
		npctalk "This is the view of the Rgan...! I have no feet!","Lehar#e19ms11",bc_self;
		npctalk "I thought I was prepared, it's more unpleasant than I expected.","Horuru#e19ms11",bc_self;
		next;
		mes "[Lazy]";
		mes "Everyone transformed safely. I don't think there are any side effects.";
		mes "We can use this with confidence.";
		npctalk "This guy, he used us for experiments!!!","Lehar#e19ms11",bc_self;
		unittalk getcharid(3),strcharinfo(0) + " : Don't try to understand him. You will be tired of life in just seconds.",bc_self;
		npctalk "Adventurer, that's too much. For the amount of time we've been together!","Lazy#e19ms11",bc_self;
		next;
		select("Hurry up and transform too.");
		specialeffect EF_FOOD06,AREA,"Lazy#e19ms11";
		cloaknpc("Lazy#e19ms11",true,getcharid(0));
		cloaknpc("Lazy#e19ms12",false,getcharid(0));
		mes "[Horuru]";
		mes "I hope everyone transformed safely, Uh... Uh...?";
		next;
		mes "[Miriam]";
		mes "Is there a problem?";
		next;
		mes "[Horuru]";
		mes "The adventurer, and the person over there is the problem.";
		next;
		mes "[Lazy]";
		mes "Me?? What's the problem? Didn't I do a good job at transforming?";
		next;
		mes "[Horuru]";
		mes "The problem is your grade. The Rgans appearance changes base on the amount of their mana, the more they are born with it, the more they look like a human being.";
		next;
		select("Why does Lazy's look like a human?");
		mes "[Miriam]";
		mes "And you adventurer, you look like a senior-grade, I think? and we are intermediate?";
		next;
		mes "[Lehar]";
		mes "I think everyone turned into intermediate-grade, ^0000CDAdventurer turned into a senior, Lazy turned into the superior-grade^000000.";
		mes "How many scrolls did you bring?";
		next;
		mes "[Horuru]";
		mes "There are only a few senior and superior-grade here, so everyone knows their faces!";
		mes "Anyways, won't we be found out if a suddenly a new senior and superior class appeared?";
		npctalk "Oh, that would be suspicious.","Lehar#e19ms11",bc_self;
		next;
		mes "[Lazy]";
		mes "Ha! Really! This is it! I didn't even thought of this as a problem since its good for us!";
		mes "I won't probably experience it again. Adventurer, aren't you glad? That I came with you?";
		unittalk getcharid(3),"Lazy, I don't think it's the time to think like that.",bc_self;
		next;
		mes "[Horuru]";
		mes "It doesn't matter if we've turned into intermediate Rgans, we'll just leave the two of them here and go alone.";
		next;
		mes "[Lazy]";
		mes "You can't do that! What do you think I'm here for? I have to get in there at all cost.";
		next;
		mes "[Miriam]";
		mes "Then... how about pretender to be ^0000CDnewborns^000000? It's not like they're monitoring the eggs.";
		next;
		mes "[Horuru]";
		mes "... That's not a bad idea. Okay, so you two can pretend to be newborn Rgans. You can test it moderately on the entrance.";
		next;
		mes "[Lazy]";
		mes "Haha! I can't believe that I'm going to pretend to be a newborn at this age. Hahaha! Adventurer? Starting from now we are 1 year old.";
		npctalk "Why do you like it?","Lehar#e19ms11",bc_self;
		npctalk "Right?","Miriam#e19ms11",bc_self;
		next;
		mes "[Horuru]";
		mes "Then, let's carefully enter one by one.";
		cloaknpc("Lehar#e19ms12",true,getcharid(0));
		sleep2 1000;
		cloaknpc("Miriam#e19ms12",true,getcharid(0));
		sleep2 1000;
		cloaknpc("Lazy#e19ms12",true,getcharid(0));
		next;
		mes "[Horuru]";
		mes "Adventurer, go ahead and enter the <NAVI>[cave]<INFO>jor_dun02,263,170,0,101,0</INFO></NAVI>.";
		completequest 17619;
		setquest 17620;
		getitem 1000608,25;
		close2;
		cloaknpc("Horuru#e19ms12",true,getcharid(0));
		navigateto("jor_dun02",263,170);
		end;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17619) == 1";
end;
}

jor_back3,99,315,1	script	Lehar#e19ms11	4_EP19_LEHAR,{
	cutin "ep19_lehar01.png",2;
	mes "[Lehar]";
	mes "Is this the place? I think we should transform ourselves here!";
	close3;
}
	
jor_back3,96,316,7	script	Miriam#e19ms11	4_EP18_MIRIAM,{
	cutin "ep18_miriam_01.png",0;
	mes "[Miriam]";
	mes "This in area that Horuru knows very well, so it's better to stop here.";
	close3;
}


jor_back3,97,318,5	script	Lazy#e19ms11	4_EP19_LAZY,{
	cutin "ep19_leizi01.png",1;
	mes "[Lazy]";
	mes "Is the one in front of us the Rgans nest?";
	close3;
}
	
jor_back3,99,318,3	duplicate(dummynpc2)	Horuru#e19ms12	21529
jor_back3,97,318,5	duplicate(dummynpc2)	Lazy#e19ms12	4_EP19_RGAN_SR3
jor_back3,99,315,1	duplicate(dummynpc2)	Lehar#e19ms12	21529
jor_back3,96,316,7	duplicate(dummynpc2)	Miriam#e19ms12	21529

jor_dun02,263,170,0	script	#e19ms20	HIDDEN_WARP_NPC,5,5,{
	end;
	
OnTouch:
	if(isbegin_quest(17620) == 1){
		cloaknpc("Miriam#e19ms20",false,getcharid(0));
		cloaknpc("Horuru#e19ms21",false,getcharid(0));
		cloaknpc("Lazy#e19ms21",false,getcharid(0));
		cloaknpc("Lehar#e19ms21",false,getcharid(0));
		if(!191_scroll_check()){
			mes "[Miriam]";
			mes "Adventurer, you'd better transform into a Rgan.";
			close;
		}
	}
	if(isbegin_quest(17627) == 1)
		cloaknpc("Miriam#e19ms20",false,getcharid(0));
end;
}

jor_dun02,263,170,3	script	Miriam#e19ms20	21529,{
	if(!191_scroll_check()){
		mes "[Miriam]";
		mes "Adventurer, you'd better transform into a Rgan.";
		close;
	}
	if(isbegin_quest(17620) == 1){
		mes "[Miriam]";
		mes "We were able to come in somehow... No one really recognize us.";
		next;
		mes "[Lehar]";
		mes "Oh, I think it's a bit late to ask this, but how does a Rgan talk?";
		next;
		mes "[Horuru]";
		mes "Don't worry too much about that. It's very similar to what humans speak.";
		npctalk "Things are going well.","Lazy#e19ms21",bc_self;
		next;
		mes "[Horuru]";
		mes "However, be careful on what you say, you might spill something from the outside unknowingly. It would be suspicious that a newborn Rgan doesn't have any knowledge about the inside but know something from the outside.";
		next;
		mes "[Horuru]";
		mes "Just, don't say anything if you don't know what to say.";
		npctalk "Hmmm... that's a bit difficult.","Lazy#e19ms21",bc_self;
		next;
		mes "[Horuru]";
		mes "Act like a fool. Like a human who doesn't know anything. Fortunately, Rgans have great respect towards their seniors and higher classes, don't worry too much.";
		next;
		mes "[Lazy]";
		mes "*Cries*-";
		unittalk getcharid(3),"Did you eat something wrong today?",bc_self;
		next;
		mes "[Lehar]";
		mes "What will happen to our clothes when the transformation goes away?";
		next;
		mes "[Lazy]";
		mes "Oh, I asked them about that before we came here, they said they were dressed up like before transforming. Happy now? Don't worry about it too much, nephew.";
		next;
		mes "[Horuru]";
		mes "It would be better to act separately from here. I'll go to the right from here.";
		next;
		mes "[Lazy]";
		mes "Then me and the adventurer will go left.";
		next;
		mes "[Miriam]";
		mes "Then, I'll look around over there.";
		next;
		mes "[Horuru]";
		mes "Well, let's go-";
		next;
		cloaknpc("Rgan#e19ms21",false,getcharid(0));
		emotion ET_HUK,getnpcid(0,"Miriam#e19ms20");
		emotion ET_HUK,getnpcid(0,"Horuru#e19ms21");
		emotion ET_HUK,getnpcid(0,"Lazy#e19ms21");
		emotion ET_HUK,getnpcid(0,"Lehar#e19ms21");
		mes "[Rgan Priest]";
		mes "What's the fuss here, huh???!";
		next;
		mes "[Rgan Priest]";
		mes "A senior, and a bishop? Why is the bishop here??";
		next;
		emotion ET_HUK,getnpcid(0,"Horuru#e19ms21");
		cloaknpc("Rgan#e19ms22",false,getcharid(0));
		mes "[Rgan Priest]";
		mes "What's the fuss over here? Why's the bishop over here??";
		npctalk "What's the fuss over here? Oh!!! A bishop?","Rgan#e19ms22",bc_self;
		next;
		emotion ET_HUK,getnpcid(0,"Rgan#e19ms22");
		mes "[Horuru]";
		mes "Oh! The bishop were just born. I was watching it.";
		npctalk "Born?","Rgan#e19ms22",bc_self;
		next;
		mes "[Rgan Priest]";
		mes "This isn't where we put superior eggs, is it?";
		npctalk "A priest and a bishop?","Rgan#e19ms22",bc_self;
		next;
		mes "[Rgan Priest]";
		mes "Hey, was it born in non-superior hatchery? How long has it been?";
		next;
		mes "[Rgan Priest]";
		mes "It shouldn't stay here, ^0000CDlet's take it at the upper level^000000.";
		next;
		mes "[Horuru]";
		mes "I will check again if there is a Priest class among the other eggs.";
		npctalk "Oh, goodluck!","Rgan#e19ms21",bc_self;
		npctalk "Come, let's go. Childrens! No, dear childrens.","Rgan#e19ms22",bc_self;
		npctalk "*Cries*!","Lazy#e19ms21",bc_self;
		sleep2 1500;
		npctalk "Eh, why? Isn't this one kind of weird?","Rgan#e19ms22",bc_self;
		npctalk "A bishop being born. Have you ever seen a bishop being born?","Rgan#e19ms21",bc_self;
		sleep2 1000;
		npctalk "There's no way I would see such a thing.","Rgan#e19ms22",bc_self;
		next;
		mes "[Rgan Priest]";
		mes "Let's go to the <NAVI>[upper level]<INFO>jor_nest,185,49,0,101,0</INFO></NAVI>.";
		next;
		sleep2 1000;
		navigateto("jor_nest",185,49);
		mes "[Rgan Priest]";
		mes "Oh, please understand our treatment. We haven't figure out yet if you're a friend or foe";
		completequest 17620;
		setquest 17621;
		getitem 1000608,25;
		close2;
		navigateto("jor_nest",185,49);
		cloaknpc("Lazy#e19ms21",true,getcharid(0));
		cloaknpc("Rgan#e19ms21",true,getcharid(0));
		cloaknpc("Rgan#e19ms22",true,getcharid(0));
		end;
	}
	if(isbegin_quest(17621) == 1){
		mes "[Miriam]";
		mes "What are you doing here, please follow the <NAVI>[Rgan]<INFO>jor_nest,185,49,0,101,0</INFO></NAVI>.";
		close2;
		navigateto("jor_nest",185,49);
		cloaknpc("Miriam#e19ms20",true,getcharid(0));
		cloaknpc("Horuru#e19ms21",true,getcharid(0));
		cloaknpc("Lehar#e19ms21",true,getcharid(0));
		end;
	}
	if(isbegin_quest(17627) == 1){
		mes "[Miriam]";
		mes "Adventurer?";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "How do I get an eggshell... She probably won't talk to me if I don't have it.";
		next;
		mes "[Miriam]";
		mes "Adventurer!";
		next;
		select("...?");
		mes "[Miriam]";
		mes "It's me, Miriam.";
		next;
		select("How did you recognize me?");
		unittalk getcharid(3),strcharinfo(0) + " : " + "I can't tell the difference.",bc_self;
		mes "[Miriam]";
		mes "That, you can tell by look at how awkward you walk. Horuru and Lehar went to a different place, so the only other one with that awkward appearance is you.";
		next;
		select("Oh, I see.");
		mes "[Miriam]";
		mes "What's going on?";
		next;
		select("I need senior-grade eggshell.");
		mes "[" + strcharinfo(0) + "]";
		mes "I made contact with an Illusion, and it seemed that they were already here beforehand. She told me to bring my senior-grade eggshell.";
		next;
		mes "[Miriam]";
		mes "But, This is the lower-level hatchery. It seems that the ^0000CDhigher-grade eggs are classified separately^000000.";
		next;
		mes "[Miriam]";
		mes "Oh, Horuru and Lehar is over <NAVI>[there]<INFO>jor_nest,23,58,0,101,0</INFO></NAVI>. Maybe we'll be able to get get help.";
		mes "Go ahead.";
		completequest 17627;
		setquest 17628;
		questinfo_refresh;
		close;
	}
	if(isbegin_quest(17628) == 1){
		mes "[Miriam]";
		mes "Horuru and Lehar is in the <NAVI>[advanced area]<INFO>jor_nest,23,58,0,101,0</INFO></NAVI>, go and maybe we'll be able to get help.";
		close;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17620) == 1";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17627) == 1";
end;
}

jor_dun02,256,169,5	duplicate(dummynpc2)	Rgan#e19ms21	4_EP19_RGAN_R1
jor_dun02,254,168,5	duplicate(dummynpc2)	Rgan#e19ms22	4_EP19_RGAN_R3

jor_dun02,263,167,1	script	Lazy#e19ms21	4_EP19_RGAN_SR3,{
	if(isbegin_quest(17620) == 1){
		mes "[Lazy]";
		mes "Is this where I was born?";
		close;
	}
	end;
}

jor_dun02,261,166,7	script	Horuru#e19ms21	21529,{
	if(isbegin_quest(17620) == 1){
		mes "[Horuru]";
		mes "It's my first time coming into a Rgans nest.";
		close;
	}
	end;
}

jor_dun02,263,165,1	script	Lehar#e19ms21	21529,{
	if(isbegin_quest(17620) == 1){
		mes "[Lehar]";
		mes "It's so creepy here.";
		close;
	}
	end;
}


jor_dun02,18,31,0	script	#to_jor_nest	WARPNPC,1,1,{
	end;
	
OnTouch:
	if(isbegin_quest(17621) == 0) end;
	warp "jor_nest",278,18;
end;
}

jor_nest,185,49,3	script	Rgan Priest#e19ms31	4_EP19_RGAN_R1,{
	if(!191_scroll_check()){
		mes "[Rgan Priest]";
		mes "What, a dirty human being is in here! Go suffer in the cold!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	if(isbegin_quest(17621) == 1){
		cloaknpc("Lazy#e19ms31",false,getcharid(0));
		mes "[Rgan Priest]";
		mes "Look who I brought!";
		next;
		mes "[Rgan Priest]";
		mes "A bishop was born!!!";
		next;
		mes "[Rgan Priest]";
		mes "It has been a while, they are ^0000CDrarely born these days^000000.";
		next;
		mes "[Rgan Priest]";
		mes "Is that all you want to say? He was born in the lower level!";
		next;
		mes "[Rgan Priest]";
		mes "What?? How?";
		next;
		mes "[Rgan Priest]";
		mes "It's, a ^0000CDmiracle^000000! It's a sign of Jormungandr's resurrection!";
		next;
		mes "[Rgan Priest]";
		mes "Bring him here. Let me see who it is. I need to know his face.";
		cloaknpc("Rgan Bishop#e19ms31",false,getcharid(0));
		next;
		mes "[Rgan Bishop]";
		mes "What was born?";
		next;
		mes "[Rgan Priest]";
		mes "Bishop!";
		npctalk "Bishop!","Rgan Priest#e19ms31",bc_self;
		npctalk "Bless you!","Rgan Priest#e19ms32",bc_self;
		npctalk "Bless you!","Rgan Priest#e19ms33",bc_self;
		next;
		mes "[Rgan Priest]";
		mes "A new bishop has been born in the lower level.";
		next;
		mes "[Rgan Bishop]";
		mes "Ho... in the lower level? Let's see... I haven't seen that priest next to him before?";
		next;
		mes "[Rgan Priest]";
		mes "Oh, they were born in the lower level together...";
		next;
		mes "[Rgan Bishop]";
		mes "Really? Let's talk to ^0000CDSarekgand^000000 and ask him to take this one to Lasgand. To ask them to ^0000CDbless him with a name^000000.";
		next;
		mes "[Rgan Bishop]";
		mes "Where's Sarekgand?";
		next;
		mes "[Rgan Priest]";
		mes "Recently, he's been going to the ^0000CDhuman zone^000000 that's managed by Lasgand.";
		next;
		mes "[Rgan Bishop]";
		mes "Then, I'll bring him over to Sarekgand.";
		mes "You two, you should get used to this treatment before you get baptized, got it?";
		next;
		mes "[Rgan Bishop]";
		mes "This is a square. This is the place where priest-grade or higher Rgans live.";
		unittalk getcharid(3),strcharinfo(0) + " : Priest-Grade Rgan?",bc_self;
		next;
		mes "[Rgan Priest]";
		mes "I'll tell you the details on the way.";
		mes "Here, come with me. Yes, just move like that.";
		next;
		mes "[Lazy]";
		mes "*Stretch*?";
		next;
		mes "[Rgan Bishop]";
		mes "More gently. A bit more. There you go. You already like like someone who's been walking for decades. Now, follow <NAVI>[me]<INFO>jor_nest,196,202,0,101,0</INFO></NAVI>. This place is complicated, it's easy to get lost.";
		next;
		mes "[Rgan Priest]";
		mes "The lord Jormungandr is with you!";
		next;
		mes "[Rgan Priest]";
		mes "The blessing of Jormungandr!!";
		npctalk "Jormungandr is with you!!","Rgan Priest#e19ms31",bc_self;
		npctalk "The blessing of Jormungandr!!","Rgan Priest#e19ms32",bc_self;
		completequest 17621;
		setquest 17622;
		questinfo_refresh;
		close2;
		navigateto("jor_nest",196,202);
		cloaknpc("Rgan Bishop#e19ms31",true,getcharid(0));
		cloaknpc("Lazy#e19ms31",true,getcharid(0));
		end;
	}
	if(isbegin_quest(17622) == 1){
		mes "[Rgan Priest]";
		mes "Go follow the <NAVI>[bishop]<INFO>jor_nest,196,202,0,101,0</INFO></NAVI>. It's going to be a great opportunity.";
		close2;
		navigateto("jor_nest",196,202);
		end;
	}
	mes "[Rgan Priest]";
	mes "Have you adjusted to the life here?";
	close;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17621) == 1";
end;
}

jor_nest,182,50,5	duplicate(dummynpc2)	Lazy#e19ms31	4_EP19_RGAN_SR3
jor_nest,180,48,5	duplicate(dummynpc2)	Rgan Bishop#e19ms31	4_EP19_RGAN_SR2

jor_nest,182,45,7	script	Rgan Priest#e19ms32	4_EP19_RGAN_R2,{
	mes "[Rgan Priest]";
	mes "It won't be difficult to adjust here. Ask anyone if you find anything difficult.";
	close;
}

jor_nest,186,46,1	script	Rgan Priest#e19ms33	4_EP19_RGAN_R3,{
	mes "[Rgan Priest]";
	mes "What? Do you have any question?";
	close;
}

jor_nest,196,202,5	script	Rgan Bishop#e19ms41	4_EP19_RGAN_SR2,{
	if(!191_scroll_check()){
		mes "[Rgan Bishop]";
		mes "Who brought a human here!!! Get out now!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	if(isbegin_quest(17622) == 1){
		cloaknpc("Lazy#e19ms41",false,getcharid(0));
		mes "[Rgan Bishop]";
		mes "I want to show you this place before we go to Lasgand. This place is an ^0000CDegg hatchery^000000. It's usually where a priest or bishop is born.";
		next;
		mes "[Rgan Bishop]";
		mes "It's where large eggs are selected, placed, and managed until it hatch.";
		mes "The place where you were born is where lesser-grades are raised. That makes you special. All of this, is a blessing from Jormungandr.";
		next;
		mes "[Rgan Bishop]";
		mes "In addition to that, the eggs of higher-grade are placed throughout the nest and managed separately. It would be nice to look around when you have the time.";
		next;
		mes "[Lazy]";
		mes "What's wrong with that thing?";
		next;
		mes "[Rgan Bishop]";
		mes "Oh, that thing. The humans built it. It's to inject more mana into the egg.";
		next;
		mes "[Lazy]";
		mes "Humans?";
		next;
		mes "[Rgan Bishop]";
		mes "Oh, you don't know. They came from beyond the barrier. They're staying here because of Lasgand's consideration. You'll easily know when you see them since they are different from us.";
		next;
		mes "[Rgan Bishop]";
		mes "Oh, only talk with humans inside the nest. The rest of them, are dirty things. So, shall we go <NAVI>[there]<INFO>jor_nest,115,187,0,101,0</INFO></NAVI> now?";
		completequest 17622;
		setquest 17623;
		questinfo_refresh;
		close2;
		navigateto("jor_nest",115,187);
		cloaknpc("Lazy#e19ms41",true,getcharid(0));
		end;
	}
	if(isbegin_quest(17623) == 1){
		mes "[Rgan Bishop]";
		mes "What are you looking at? The sight here are nice to look at, but you'd better hurry. You must take the bishop child to <NAVI>[Lasgand]<INFO>jor_nest,115,187,0,101,0</INFO></NAVI>.";
		close2;
		navigateto("jor_nest",115,187);
		end;
	}
	mes "[Rgan Bishop]";
	mes "You have to be careful not to breaks the eggs here.";
	close;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17622) == 1";
end;
}

jor_nest,193,200,5	duplicate(dummynpc2)	Lazy#e19ms41	4_EP19_RGAN_SR3

jor_nest,115,187,3	script	Rgan Bishop#e19ms51	4_EP19_RGAN_SR2,{
	if(!191_scroll_check()){
		mes "[Rgan Bishop]";
		mes "Humans shouldn't come here. Get out of this place.";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	if(isbegin_quest(17623) == 1){
		cloaknpc("Lazy#e19ms51",false,getcharid(0));
		mes "[Rgan Bishop]";
		mes "Oh, since we are passing this way. This is where the humans I spoke of a while ago reside.";
		next;
		mes "[Lazy]";
		mes "I want to go in.";
		next;
		mes "[Rgan Bishop]";
		mes "Don't go in there carelessly. They have a lot of dangerous stuffs.";
		next;
		mes "[Rgan Bishop]";
		mes "Honestly, it doesn't feel nice to have ^0000CDdirty humans^000000 stay here, we are only letting them because of Lasgand's consideration.";
		next;
		mes "[Rgan Bishop]";
		mes "I'm sure he has a meaningful thought in mind. He is different from us.";
		next;
		mes "[Rgan Bishop]";
		mes "Anyway, please don't go in this place. There are a lot of ^0000CDweird devices^000000. Things you better not knowing. Even if it's not that bad, it would be better not to be involved with humans.";
		next;
		mes "[Rgan Bishop]";
		mes "We've been delayed for too long. Now, we need to hurry <NAVI>[there]<INFO>jor_nest,53,244,0,101,0</INFO></NAVI>.";
		completequest 17623;
		setquest 17624;
		close2;
		navigateto("jor_nest",53,244);
		cloaknpc("Lazy#e19ms51",true,getcharid(0));
		end;
	}
	if(isbegin_quest(17624) == 1){
		mes "[Rgan Bishop]";
		mes "We've been delayed too long. The sight is good, but we should hurry. We need to bring the young bishop to <NAVI>[Lasgand]<INFO>jor_nest,53,244,0,101,0</INFO></NAVI>.";
		close;
	}
	mes "[Rgan Bishop]";
	mes "It's better not to go in here recklessly. There are stories about getting peeled alive.";
	close;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17623) == 1";
end;
}

jor_nest,118,187,3	duplicate(dummynpc2)	Lazy#e19ms51	4_EP19_RGAN_SR3

jor_nest,53,244,1	script	Rgan Bishop#e19ms61	4_EP19_RGAN_SR2,{
	if(!191_scroll_check()){
		mes "[Rgan Bishop]"; //TODO GET ORIGINAL DIALOGUE
		mes "Humans shouldn't come here. Get out of this place.";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	if(isbegin_quest(17624) == 1){
		cloaknpc("Lazy#e19ms61",false,getcharid(0));
		mes "[Rgan Bishop]";
		mes "Now, remember the way to this place, okay? This place ^0000CDis our home and it's a holy land blessed by Jormungandr^000000.";
		next;
		select("Holy land?");
		mes "[Rgan Bishop]";
		mes "Yes. It's where the mana of Jormungandr resides. You were with blessing of that mana.";
		next;
		mes "[Rgan Bishop]";
		mes "Our lives comes from Jormungandr, proving our faith to the life given, we were blessed with such mana.";
		next;
		mes "[Rgan Bishop]";
		mes "That's why we priests and bishops must inform these lands of Jormungandr's resurrection.";
		next;
		mes "[Rgan Bishop]";
		mes "The lord Jormungandr is always with us. We should be grateful to the life that was given to us.";
		next;
		mes "[Rgan Bishop]";
		mes "Then, I'm sure you've already learned the geography here, you'll be able to return here on your own, you follow me.";
		next;
		mes "[Rgan Bishop]";
		mes "We have to meet Lasgand.";
		next;
		select("Lasgand?");
		mes "[Rgan Bishop]";
		mes "It should be Mr. Lasgand. He is our teacher. He has been with Jormungandr since the beginning of time and the guardian of this land.";
		next;
		mes "[Rgan Bishop]";
		mes "He's the reason we overcame the darkness, survived and was able to prosper this much.";
		next;
		mes "[Lazy]";
		mes "Haha! It's an honor that I can meet him after being born! Where can I meet him?";
		next;
		mes "[Rgan Bishop]";
		mes "The place where Lasgand resides. Not everyone can meet him, only authorized can come and go enter the cult's room.";
		next;
		mes "[Rgan Bishop]";
		mes "But today a Rgan of the superior-grade is born and should be baptized with his blessings.";
		mes "He always take care of us.";
		cloaknpc("Sarekgand#e19ms61",false,getcharid(0));
		next;
		npctalk "I can't believe I'm going to pick up a human being with my own body!!","Sarekgand#e19ms61",bc_self;
		mes "[Rgan Bishop]";
		mes "Oh! Sarekgand! I was on my way to see you!! Come here, come!!";
		next;
		mes "[Sarekgand]";
		mes "What's all of this about? Making such noise like those vile things.";
		next;
		mes "[Rgan Bishop]";
		mes "Oh, that was a bad way to say it. But today is a good day, so I'll let it slide.";
		npctalk "Good day?","Sarekgand#e19ms61",bc_self;
		next;
		mes "[Rgan Bishop]";
		mes "Look. A bishop-grade Rgan was born in the lower level! A priest was also born with him.";
		mes "They should be baptized by Lasgand. That's why I was looking for you.";
		npctalk "Ho...? It has been a long time. And also in the lower level?","Sarekgand#e19ms61",bc_self;
		next;
		mes "[Sarekgand]";
		mes "Lasgand would be pleased. We are in deep waters lately, so that will be a good news.";
		cloaknpc("Bagot#e19ms61",false,getcharid(0));
		npctalk "What are you talking about?","Bagot#e19ms61",bc_self;
		next;
		mes "[Sarekgand]";
		mes "^0000CDHuman^000000! Perfect timing. Lasgand is looking for you.";
		emotion ET_SURPRISE;
		emotion ET_SURPRISE,getnpcid(0,"Lazy#e19ms61");
		npctalk "That's...!","",bc_self;
		next;
		cutin "ep18_bagot_01.png",2;
		mes "[Bagot]";
		mes "Okay. Skekangand, is he inside?";
		next;
		emotion ET_ANGER,getnpcid(0,"Sarekgand#e19ms61");
		cutin "",255;
		mes "[Sarekgand]";
		mes "How dare...! Being Lasgand's closest friend, my name...!";
		next;
		cutin "ep18_bagot_01.png",2;
		mes "[Bagot]";
		mes "Oh, was it Skakan? No? But, I tried to call your name, are you unsatisfied? What was your name again?";
		next;
		cutin "",255;
		mes "[Sarekgand]";
		mes "This cheeky...!";
		next;
		cutin "ep18_bagot_02.png",2;
		mes "[Bagot]";
		mes "So, the name? Should I sing out your name again?";
		next;
		cutin "",255;
		mes "[Sarekgand]";
		mes "It's Sarekgand. If you get it wrong again, Lasgand's favor for you will surely fade.";
		npctalk "As arrogant as I've heard...","Rgan Bishop#e19ms61",bc_self;
		next;
		cutin "ep18_bagot_01.png",2;
		mes "[Bagot]";
		mes "What does it matter. Just show me around. I'm a busy person.";
		next;
		cutin "",255;
		mes "[Sarekgand]";
		mes "Tsk. If today wasn't a happy day of a bishop being born, your hideous head would have been on the ground.";
		mes "Now, you, stay here and the superior follow me.";
		next;
		cutin "ep18_bagot_03.png",2;
		mes "[Bagot]";
		mes "Ho, a superior Rgan? Was it born? At this time?";
		next;
		cutin "",255;
		mes "[Sarekgand]";
		mes "You can't experiment with a superior-grade Rgans, so don't show interest.";
		next;
		cutin "ep18_bagot_01.png",2;
		mes "[Bagot]";
		mes "That hasn't been decided yet. There's also that senior Rgan.";
		next;
		select("Me?");
		mes "[Bagot]";
		mes "Yeah, you. If you don't have anywhere to go, why don't you go to the place in front of me? You should atleast get your ^0000CDhealth check from Juncea^000000. Bagot and I will be back soon.";
		next;
		select("Juncea?");
		cutin "",255;
		mes "[Sarekgand]";
		mes "Come, Bishop, let's go.";
		next;
		mes "[Rgan Bishop]";
		mes "The Lord Jormungandr is with you!";
		next;
		mes "[Sarekgand]";
		mes "Blessings from Jormungandr!";
		next;
		cloaknpc("Bagot#e19ms61",true,getcharid(0));
		cloaknpc("Sarekgand#e19ms61",true,getcharid(0));
		cloaknpc("Lazy#e19ms61",true,getcharid(0));
		mes "[" + strcharinfo(0) + "]";
		mes "Hmm... a health check at the residence of an <NAVI>[Illusion]<INFO>jor_nest,22,141,0,101,0</INFO></NAVI>...";
		completequest 17624;
		setquest 17625;
		questinfo_refresh;
		close2;
		navigateto("jor_nest",22,141);
		end;
	}
	if(isbegin_quest(17625) == 1){
		mes "[Rgan Bishop]";
		mes "Didn't I say to go meet a <NAVI>[human]<INFO>jor_nest,22,141,0,101,0</INFO></NAVI>? Go on then.";
		close2;
		navigateto("jor_nest",22,141);
		end;
	}
	mes "[Rgan Bishop]";
	mes "The atmosphere is a bit chaotic these days.";
	close;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17624) == 1";
end;
}

jor_nest,55,244,1	duplicate(dummynpc2)	Lazy#e19ms61	4_EP19_RGAN_SR3
jor_nest,51,246,5	duplicate(dummynpc2)	Sarekgand#e19ms61	4_EP19_RGAN_SR1
jor_nest,56,246,3	duplicate(dummynpc2)	Bagot#e19ms61	4_EP18_BAGOT

jor_nest,22,141,0	script	#e19ms71	HIDDEN_WARP_NPC,5,5,{
	end;
	
OnTouch:
	if(isbegin_quest(17625) == 1 || isbegin_quest(17629) == 1 || isbegin_quest(17631) == 1)
		cloaknpc("Juncea#e19ms71",false,getcharid(0));
	if(isbegin_quest(17630) == 1){
		cloaknpc("Juncea#e19ms71",false,getcharid(0));
		cloaknpc("Lazy#e19ms72",false,getcharid(0));
		cloaknpc("Bagot#e19ms71",false,getcharid(0));
		cloaknpc("Sarekgand#e19ms71",false,getcharid(0));	
	}
	
end;
}

jor_nest,22,141,5	script	Juncea#e19ms71	4_EP19_JUNCEA,{
	if(!191_scroll_check()){
		mes "[Juncea]";
		mes "An experiment? No. You're not supposed to be here. Isn't there a certain area where you should be?";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	if(isbegin_quest(17625) == 1){
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "Another one came without letting me know. Next time, I need to install mines...";
		mes "... it's annoying.";
		next;
		select("Juncea?");
		mes "[Juncea]";
		mes "Who dares come in and call out my name?";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "I was born today. They want me to get a health checkup. By the way, what's a health checkup?";
		next;
		cutin "ep19_juncea02.png",2;
		mes "[Juncea]";
		mes "What...?";
		mes "You were born today?";
		mes "You're a senior Rgan. The eggs in the hatchery showed no signs of such.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "I was born in the lower level. They say it's a blessing.";
		mes "The bishop who was born with me went somewhere to be baptized.";
		next;
		cutin "ep19_juncea02.png",2;
		mes "[Juncea]";
		mes "A senior and a top-grade Rgan were born in the lower level? There is no way?";
		mes "The remaining mana there... is far off...";
		next;
		select("I'm here for a health checkup.");
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "Oh! They told you to come here to get check out, right? Come here. In front of me. I need to check your mana.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "What's all of this? It's different from other places. I'm a little scared.";
		next;
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "Ask someone else.";
		next;
		select("......");
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "......Okay. If I told you, will you still get a health checkup?";
		unittalk getcharid(3),strcharinfo(0) + " : " + "Humans are scary.",bc_self;
		npctalk "No, I'm not scary. It's because were not that close.","Juncea#e19ms71",bc_self;
		next;
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "This is a place to experiment with different things. Where we study. Now, I will do a test.";
		mes "I have to do a test, if you're healthy or not, got it?";
		unittalk getcharid(3),strcharinfo(0) + " : " + "Aren't you afraid of being close to me?",bc_self;
		npctalk "Not really.","Juncea#e19ms71",bc_self;
		sleep2 1500;
		unittalk getcharid(3),strcharinfo(0) + " : " + "Then let's become close.",bc_self;
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "Sigh- ...Sure. Now raise your arm.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "You look different from other people. You're different from me.";
		next;
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "That's because I'm a human being. A different race from another place. Can I see your eyes?";
		next;
		select("Another place?");
		unittalk getcharid(3),strcharinfo(0) + " : " + "It's bright...",bc_self;
		mes "[Juncea]";
		mes "Not from this land. But outside. A very far away place.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "How did you get here from somewhere far away. My body starts to hurt even If I walk just a little.";
		next;
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "I came here on an airship. A long time ago.";
		next;
		select("Airship? what's that?");
		mes "[Juncea]";
		mes "A big ship that's powered by the Heart of Ymir, It... can... fly in the sky. Don't ask me what a ship is. You'll know when you see it..";
		mes "You can look around as a Senior Rgan.";
		next;
		select("How do I get there?");
		mes "[Juncea]";
		mes "If you go that way, you'll see it. If you don't know that way, ask someone else.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "How do I get out? How does it feel outside?";
		next;
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "Go out and see for yourself. You can get out of this laboratory and go downstairs. There's only one entrance, you'll be able to get out even if you just wander around.";
		next;
		select("Only one entrance?");
		mes "[Juncea]";
		mes "Yes. There's only one. Everyone comes in and out of there. I heard that you can't just drill holes here and there.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "I saw someone who looks like you.";
		next;
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "Bagot. He's a new guy who just came recently.";
		unittalk getcharid(3),strcharinfo(0) + " : " + "A friend?",bc_self;
		npctalk "We used to share a laboratory together, but work on a different field. We haven't talked much. I'm not interested. And I don't care.","Juncea#e19ms71",bc_self;
		next;
		mes "[Juncea]";
		mes "Oh- around here..";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "Oh- Wah!";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "I guess that was too deep. You'd better take a break.";
		completequest 17625;
		setquest 17626;
		questinfo_refresh;
		close3;
	}
	if(isbegin_quest(17626) == 1){
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "Have you rested enough? Should we continue?";
		next;
		select("What are you doing here?");
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "For research.";
		next;
		select("What's research?");
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "Learning about what you're curious about.";
		mes "Look, here. Can you be quiet for ten minutes?";
		next;
		mes "[Juncea]";
		mes "No, point you finger out.";
		unittalk getcharid(3),strcharinfo(0) + " : " + "......",bc_self;
		next;
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "Finger.";
		unittalk getcharid(3),strcharinfo(0) + " : " + "............",bc_self;
		next;
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "What's with your expression.";
		unittalk getcharid(3),strcharinfo(0) + " : " + "....................",bc_self;
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "... Sigh- I see. What are you curious about?";
		next;
		select("What I'm curious about?");
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "Like, This place and Rgans like you. About Mana.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "Then, are you researching me too? Is that why I have to get a health checkup?";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "No, I can't do that. I promised them when I came here.";
		mes "They'll support our research, but we're not to use a high-grade Rgan in the laboratory.";
		next;
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "In this place your status is determined by the natural mana you have. Superior, senior, intermediate, and so on.";
		mes "In your words, these are called blessings.";
		next;
		mes "[Juncea]";
		mes "Blessed seniors and above are the inhabitants of this place. In other words.";
		mes "They can think, so everyone gets nervous when we use on of the seniors for experiments.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "I see. I'm a blessed senior.";
		mes "Did you already fill your curiousity by researching?";
		next;
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "No, not yet. There is a connection between you and the mana here, and if I want to develop it futher, No... It can't be.";
		next;
		mes "[Juncea]";
		mes "However, recently it is said that higher-grade Rgans are rarely born.";
		mes "It's because there remaining mana is almost dry...";
		mes "Maybe...?";
		unittalk getcharid(3),strcharinfo(0) + " : " + "Your eyes are shining.",bc_self;
		next;
		mes "[Juncea]";
		mes "I can't use you for an experiment, but I can use the egg where you came out of.";
		mes "Bring me your eggshell. I'll study the eggs and the remaining mana on it.";
		next;
		select("Is there any left?");
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "Go check it out. It's your job. I'll be staying here.";
		unittalk getcharid(3),strcharinfo(0) + " : " + "I want to stay here more.",bc_self;
		next;
		mes "[Juncea]";
		mes "Go get the <NAVI>[eggshell]<INFO>jor_dun02,263,170,0,101,0</INFO></NAVI> with the blessing of the snake god.";
		completequest 17626;
		setquest 17627;
		questinfo_refresh;
		close3;
	}
	if(isbegin_quest(17629) < 1){
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "Go get the eggshell. I'm a little busy.";
		close3;
	}
	if(isbegin_quest(17629) == 1){
		mes "[Juncea]";
		mes "The medium that connects the mana...";
		mes "Who is it? Who came in without saying anything?";
		next;
		select("I brought the eggshells.");
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "Oh, it's you. Is this your eggshells?";
		unittalk getcharid(3),strcharinfo(0) + " : " + "Yes. This is all what's left.",bc_self;
		npctalk "It's okay. That is enough.","Juncea#e19ms71",bc_self;
		next;
		cutin "ep19_juncea02.png",2;
		specialeffect EF_AGIUP3,AREA,"Juncea#e19ms71";
		mes "[Juncea]";
		mes "The remaining mana is quite high";
		mes "I think it just hatched. But the shape is different from other eggshells that hatched.";
		next;
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "Is it because it's a senior's eggshell from the lower level?";
		mes "There must be a little mana left in your body. How come a senior was born?";
		next;
		select("My body? Mana?");
		mes "[Juncea]";
		mes "I guess no one told you.";
		mes "Ask another Rgan later.";
		next;
		select(".......");
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "Stop looking at me like that.";
		mes "Ha- fine.";
		next;
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "This place is a huge mana in itself.";
		mes "Rgans who are naturally drawn to mana by instinct are born here.";
		next;
		mes "[Juncea]";
		mes "It's been doing this for a long time. A very long time. But the source aren't infinite, right?";
		mes "The mana of this place is already running out.";
		next;
		select("Is it a big deal?");
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "Yes, it is. That's the reason senior Rgans weren't born recently, and you were born. In the lower levels too.";
		mes "You are something special. That's why I wanted to examine your eggshell.";
		next;
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "If I have the time, I would also like to see the superior-grade Rgan who was born with you, but that likely happen.";
		next;
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "But what will happen to the senior and superior Rgan who's born in the lower level hatchery with such little mana?";
		mes "The mana, is it coming back? Is it the final burst of mana before it runs out completely?";
		next;
		select("I don't know what you mean.");
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "You Rgans are born with mana as much as you absorb while you're still an egg .";
		next;
		mes "[Juncea]";
		mes "To be exact, the grade of the eggs are determined by the grade of the Rgan who planted the blood, but sometimes eggs abosrb a lot of mana and become a higher grade than their predecessor.";
		mes "It's the thing that you call a bishop or a priest.";
		next;
		select("How do you know that?");
		mes "[Juncea]";
		mes "My field of study is the use of mana and mana tools.";
		mes "To store mana permanently in the body. Humans aren't able to do that, but the Rgans here can.";
		next;
		mes "[Juncea]";
		mes "That's my main interest here. To amplify the remaining mana on the body, to inject mana into a lesser-grade egg to give birth to a higher-grade Rgan.";
		mes "Sending the airship beyond the barrier with their request.";
		next;
		select("Is Rgan strong? A senior?");
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "It's strong. They can control mana, and can see what humans can't.";
		mes "Didn't the humans struggled with them in the past?";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "Even a low-grade Rgan are stronger than a normal human. And you'll be able to replenish your mana here. Since there's a lot. ";
		next;
		select("I see. we're strong...");	
		cutin "ep19_juncea01.png",2;
		npctalk "Now, stand in front of me.","Juncea#e19ms71",bc_self;
		mes "[" + strcharinfo(0) + "]";
		mes "Some priests were taken to work earlier. What's that?";
		next;
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "Ah, there? That's where the low-grade Rgans are working. It's a place even an intermediate Rgans are not allowed.";
		mes "Now, I'm going to stop talking and focus on your mana waves.";
		next;
		select("I have a lot of questions.");
		mes "[Juncea]";
		mes "Rgan, there's nothing I can tell you. Ask the same question to a Rgan.";
		mes "Wait, someone's here.";
		next;
		cutin "",255;
		cloaknpc("Lazy#e19ms72",false,getcharid(0));
		mes "[Lazy]";
		mes "Tada! I'm here!";
		next;
		cloaknpc("Bagot#e19ms71",false,getcharid(0));
		cloaknpc("Sarekgand#e19ms71",false,getcharid(0));
		npctalk "Juncea, Bagot is here.","Bagot#e19ms71",bc_self;
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "What do you need?";
		delitem 1000606,1;
		completequest 17629;
		setquest 17630;
		questinfo_refresh;
		close3;
	}
	if(isbegin_quest(17630) == 1){
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "Why did Bagot bring the bishop?";
		close3;
	}
	if(isbegin_quest(17631) == 1){
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "I was here first!!";
		next;
		mes "[Juncea]";
		mes "I was here first!!";
		mes "I did more than him!!";
		next;
		mes "[Juncea]";
		mes "I was here first!!";
		mes "I did more than him!!";
		mes "I told them everything about mana!!";
		next;
		mes "[Juncea]";
		mes "I was here first!!";
		mes "I did more than him!!";
		mes "I told them everything about mana!!";
		mes "I'm more relevant!";
		next;
		cutin "",255;
		mes "[Lazy]";
		mes "This lady is a little weird. I didn't think this place was like this, what's wrong with her?";
		next;
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "Juncea.";
		mes "No, call me Juncea.";
		next;
		cutin "",255;
		mes "[Lazy]";
		mes "Juncea?";
		mes "Your name is four syllables, are you a senior? Are you an archbishop, or maybe a bishop?";
		mes "If not, it's right that the human Bagot receive more support. You have to do what your senior wants you to do.";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "<FONT SIZE = 15><B>What? That kind of kid! Better than me, what??</B></FONT>";
		next;
		cutin "",255;
		mes "[Lazy]";
		mes "Are you taking it out on us for no reason because your support was delayed?";
		next;
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "What's wrong with this one? Even if you were just born today and a superior one, Isn't it a bit strange?";
		next;
		cutin "",255;
		mes "[Lazy]";
		mes "My condition is perfectly normal. Not every Rgan is the same.";
		mes "Juncea you're a human but not the same as that human, Bagot. Juncea is human but slightly different.";
		next;
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "Come here. Maybe I should check your head first.";
		next;
		cutin "",255;
		mes "[Lazy]";
		mes "What's wrong with my head, Juncea? Even if we do a test, it's probably better than anyone here.";
		next;
		mes "[Lazy]";
		mes "Our race was at odds a long time ago, But I will accept humans, beyond species.";
		mes "we have a lot in common, so we're like siblings, don't you think? Rgan, humans. We both have livers.";
		next;
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "I was mistaken";
		mes "I should seal your mouth first before your head.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(3) + "]";
		mes "Juncea is a human! I have a question.";
		mes "Is your research different from other person before?";
		next;
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "It's different. If I were him, I would have sat here and research with me.";
		mes "I'm sure he found something that got the interest of Lasgand.";
		next;
		select("What's that?");
		mes "[Juncea]";
		mes "I don't know. Now, put this on your body.";
		next;
		cutin "",255;
		mes "[Lazy]";
		mes "I don't listen to others who are weaker than me. Be stronger than me. Then I'll listen to you.";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "Didn't you hear what they said? They went your datas. Come here.";
		mes "I won't hurt you. Just put this on and sit here for a second.";
		next;
		cutin "",255;
		mes "[Lazy]";
		mes "Does that mean that you can hurt us, can't you?";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "You're- no, alright. Come here anyway.";
		mes "Come to think of it, did you meet Lasgand with Bagot? Have you heard anything about the research of Bagot?";
		next;
		cutin "",255;
		mes "[Lazy]";
		mes "I didn't hear them. However, he showed a document with great mana on it, Lasgand supports this mana, and that he'll need support for his research to achieve something.";
		mes "Lasgand was so pleased from what he saw that he decided to support him.";
		next;
		select("What do you think it is?");
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "Well? I don't know. Like I said, we are not close.";
		mes "If it's a great mana... it's probably the Heart of Ymir. Maybe he found a way to absorb the remaining mana from this place...";
		next;
		mes "[Juncea]";
		mes "Or did he find a way to absorb the mana of the heart itself...?";
		mes "Or maybe it's about finding the body of Jormungandr that they're so obsessed with.";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "No, why didn't they just gave me the Heart of Ymir?";
		mes "Is it him who had a hard time? I arrived here first and had hard time settling! I've done everything what the Rgan asked to gain their trust!";
		npctalk "Calm down, senior-grade human. You might hurt us.","Lazy#e19ms72",bc_self;
		next;
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "...Now, it's finished. They might order you to return here later. Just come back here by then. What is your names?";
		next;
		cutin "",255;
		mes "[Lazy]";
		mes "I am Svegand. Lasgand named me.";
		next;
		select("Me, I don't have any.");
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "What? You don't have a name? Usually a high-grade Rgan were given names as soon as they were born.";
		mes "Everyone must have forgotten because a superior Rgan was born with you.";
		next;
		mes "[Juncea]";
		mes "Then... you'll be Senekiogand. There was a rule from naming Rgans, Hm... Yes. It was the numbers of syllables.";
		next;
		select("I think it's cool. What does it mean?");
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "Oh, it's the place where my laboratory... No, Nothing. You don't really have to know what it means, do you?";
		next;
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "Well, There is nothing more to do for today, so go <NAVI>[outside]<INFO>jor_nest,87,164,0,101,0</INFO></NAVI> and do your job. See you later. You must come again.";
		mes "I'll have to meet and talk with Bagot as soon as everything is sorted out.";
		completequest 17631;
		setquest 17632;
		questinfo_refresh;
		close2;
		navigateto("jor_nest",87,164);
		cloaknpc("Lazy#e19ms72",true,getcharid(0));
		cutin "",255;
		end;
	}
	if(isbegin_quest(17632) == 1){
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "Why are you still here? Go and head <NAVI>[outside]<INFO>jor_nest,87,164,0,101,0</INFO></NAVI>. You're interfering with my research. But, don't go too far and come back later.";
		close3;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17625) == 1";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17626) == 1";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17629) == 1";
end;
}

jor_nest,31,140,5	script	Juncea#ep19re1	4_EP19_JUNCEA,{
	if(isbegin_quest(16646) == 1){
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "...And another one entered with saying anything... Oh, Senekiogand. It's you.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "It's me Senekiogand. Human Juncea, what are you doing?";
		next;
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "You'll know when you see it. An Experiment.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "Is it an experiment to put a needle in your own body? Human Juncea, is your own body the experiment?";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "I'm doing this because I have no choice. I promised not to use them as test subject, ";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "Do it secretly. You can test it on me.";
		next;
		cutin "ep19_juncea02.png",2;
		mes "[Juncea]";
		mes "......You're giving me a pretty attractive suggestion?";
		mes "But I can't. You can't hide the process if you want to get the correct result.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "It's difficult. But I think you're right.";
		next;
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "Yes. I'm not doing anything wrong.";
		mes "Then, don't disturb me anymore and go out.";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "I'm tired because I'm low on sugar. No matter how important child you are, I don't have the energy to take care of you.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "What does that mean? Is that a bad thing? Juncea, is it because you experimented on your own body?";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "I can't tell you anything, really. But, I'll continue doing it, so don't disturb me.";
		next;
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "Low sugar means I have to eat something sweet.";
		mes "If you could bring me something sweet, I might able to listen to your chatter....";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "(A sweet... she's giving me order this because she knows I won't be able to get it. But if I do, I might be able to get good points from here.)";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "(Should I go back to the <NAVI>[Ice Castle]<INFO>icas_in,245,197,0,101,0</INFO></NAVI> and ask if anyone has any sweets? If I go to the adventurers dorm, will there be anyone there? As long as I get one, I can come up with excuses later.)";
		completequest 16646;
		setquest 16647;
		close;
	}
	if(isbegin_quest(16647) == 1){
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "...It's hard for me to play with you now, Senekiogand.";
		mes "What I mean is I'm busy.";
		next;
		mes "[Juncea]";
		mes "Oh, I'm low on sugar.......";
		mes "I'm sick and tired of eating the stock rations.";
		next;
		mes "[Juncea]";
		mes "I'm going to leave for a while, so don't touch anything and go out.";
		close2;
		cutin "",255;
		cloaknpc("Juncea#ep19re1",true,getcharid(0));
		end;
	}
	if(isbegin_quest(16648) == 1){
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "...Senekiogand.";
		mes "It's only you, who just come and go without saying a word.";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "Haven't I told you not to do that?";
		mes "Bagot is already ignoring me, are you going to do that too?";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "I'm not ignoring you. Didn't you said that you'd listen to me if I brought you sweets?";
		mes "I brought something sweet.";
		next;
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "Really? Why did you bring?";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "I went outside and those round furry things... He said he was an Iwin, I saw him caught somewhere and freed him.";
		mes "And he gave this to me. He said it was a sweet.";
		next;
		cutin "ep19_juncea02.png",2;
		mes "[Juncea]";
		mes "Aren't you talented?";
		mes "...Well, you're very talented. It's cold and sweet. I like it.";
		next;
		mes "[Juncea]";
		mes "So, what did you want to talk about?";
		mes "I'll give you a special listen. Well, this is quite delicious. I'm full of sugar.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "I didn't want to say anything special.";
		mes "Juncea, does human likes sweets?";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "Pff, I'm only eating it because I need it.";
		mes "Like it or hate it, it doesn't matter.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "Can't we just ask the great Lasgand?";
		mes "Lasgand said that he'd provide us what we need on behalf of Jormungandr.";
		next;
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "Oh, they don't even make these kind of snacks.";
		mes "They used to provide me with proper research materials, that's in the past though.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "Is it different now?";
		next;
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "Right now my support are tight in many ways.";
		mes "There is nothing I can do.";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "That's enough. A child shouldn't worry about that.";
		mes "I'm busy, so go now.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "I want to talk more.";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "I just can't. I'm busy.";
		mes "Bring me more of this snack.., or bring me 10 <ITEM>[Rgan Low-Grade Mana Core]<INFO>1000707</INFO></ITEM> for research.";
		next;
		mes "[Juncea]";
		mes "Otherwise don't talk to me.";
		delitem 1000846,1;
		completequest 16648;
		setquest 16649;
		questinfo_refresh;
		close3;
	}
	if(isbegin_quest(16649) == 1){
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "You're here, Senekiogand.";
		next;
		mes "[Juncea]";
		mes "I said I can't play with you because I'm busy. Did you bring more Ice Cookie? or 10 <ITEM>[Rgan Low-Grade Mana Core]<INFO>1000707</INFO></ITEM>?";
		next;
		cutin "",255;
		if(select("I brought more Ice Cookie.:I brought 10 Rgan Low-Grade Mana Core.") == 1){
			.@item_id = 1000846;
			.@amount = 1;
		} else {
			.@item_id = 1000707;
			.@amount = 10;
		}
		if(countitem(.@item_id) < .@amount){
			cutin "ep19_juncea03.png",2;
			mes "[Juncea]";
			mes "Wait, you didn't bring anything.";
			mes "Are you ignoring me?";
			close3;
		}
		cutin "ep19_juncea02.png",2;
		mes "[Juncea]";
		mes "You really brought it. You, are you sure that you're a newborn Rgan?";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "(...*Shivers*)";
		next;
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "I'm kidding. There's no way an adult Rgan could rise from the ground. If it were, it would be worth researching.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "You don't have enough to research? Is the eggshells I brought last time not enough?";
		next;
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "I cant test it for a while, but... it's probably a waste of time. To continue doing it.";
		mes "I'm sure Bagot is already getting it....";
		next;
		mes "[Juncea]";
		mes "I want to experiment with the Rgans directly, not the eggshells.";
		mes "Then, I'll be able to prove it.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "Prove what?";
		next;
		cutin "ep19_juncea02.png",2;
		mes "[Juncea]";
		mes "My hypothesis is that you can make a already Bornt Rgan stronger by injecting mana into them.";
		mes "But it's dangerous. I can't experiment without permission.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "Wow! I want to be stronger.";
		mes "It's okay to experiment with me.";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "I can't, it's a promise. Besides, it's a dangerous experiment.";
		mes "I can't experiment with you.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "Juncea is a human, If it's dangerous to a Rgan, isn't it more dangerous for you Juncea?";
		mes "Juncea, I'm worried about humans.";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "......You worry about everything.";
		mes "Now, break is over! Go play somewhere else.";
		next;
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "If you want to talk more... You know already? Not with empty hands.";
		mes "Bring me one Ice Cookie or 10 <ITEM>[Rgan Low-Grade Mana Core]<INFO>1000707</INFO></ITEM>.";
		next;
		mes "[Juncea]";
		mes "Then, I'll listen to you atleast.";
		delitem .@item_id,.@amount;
		completequest 16649;
		setquest 16650;
		questinfo_refresh;
		close3;
	}
	if(isbegin_quest(16650) == 1){
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "You really don't get tired coming and going. Do you like my laboratory, Senekiogand?";
		next;
		mes "[Juncea]";
		mes "But I can't just play with you. Which one did you bring an Ice Cookie or 10 <ITEM>[Rgan Low-Grade Mana Core]<INFO>1000707</INFO></ITEM>?";
		next;
		cutin "",255;
		if(select("I brought 1 Ice Cookie.:I brought 10 Rgan Low-Grade Mana Core.") == 1){
			.@item_id = 1000846;
			.@amount = 1;
		} else {
			.@item_id = 1000707;
			.@amount = 10;
		}
		if(countitem(.@item_id) < .@amount){
			cutin "ep19_juncea03.png",2;
			mes "[Juncea]";
			mes "Wait, you didn't bring anything.";
			mes "Then, I can't play with you.";
			close3;
		}
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "...What am I doing to this child.";
		mes "Anyway, thank you.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "Can you make time for me now?";
		next;
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "I'm already giving it to you, even now.";
		mes "I'll be busy doing experiments actually.";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "No matter how much I try to extract data, it'll all go to Bagot anyways.";
		mes "I don't care how it'll end.";
		next;
		mes "[Juncea]";
		mes "I'm already dying because I don't have any test subjects.";
		mes "But it will be the same for Bagot.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "It seems difficult to experiment these days.";
		next;
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "Yes. Neither me or Bagot have an increased in mana in our body these days. There's no progress.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "But I heard that Bagot gets a lot of support.";
		next;
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "What? There's no way.";
		mes "I already know that he gets more than I do. But I don't think it's too much.";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "I'm already receiving this much convenience... I don't think they're discriminating me that much. Besides, there is an overall shortage of resources.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "(...I brought the Ledger. I should show Juncea the proof.)";
		delitem .@item_id,.@amount;
		completequest 16650;
		setquest 16651;
		questinfo_refresh;
		close3;
	}
	if(isbegin_quest(16651) == 1){
		mes "[" + strcharinfo(0) + "]";
		mes "(...I brought the Ledger. I should show Juncea the proof.)";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "I can't. I can't believe it. He said he didn't have any resources. But that's a lie and now they're giving everything to Bagot?";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "(How should I show this? She'll be suspicious if I act that I know it well.)";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "Uhm, This~ Juncea, isn't this thing written in human text? I was happy to see the same text as written there, so I brought it.";
		next;
		cutin "ep19_juncea02.png",2;
		mes "[Juncea]";
		mes "Where did you get this?";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "I'm a young priest, so I don't ask anyone when I go anywhere. I found it while I was playing.";
		next;
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "Anyway this is an accounting ledger... You can tell by just looking at it.";
		delitem 1000845,1;
		next;
		cutin "ep19_juncea01.png",2;
		mes "[Juncea]";
		mes "........................";
		mes "........................";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "...No way. Was he cheating until now?";
		mes "Did such experimental materials really exist? and it's all going to Bagot?";
		next;
		mes "[Juncea]";
		mes "How! I can't believe this!";
		next;
		mes "[Juncea]";
		mes "I came first!!";
		mes "I did more!!";
		next;
		mes "[Juncea]";
		mes "I told them everything about mana!!!";
		mes "I'm more relevant!!!!!";
		next;
		mes "[Juncea]";
		mes "It wasn't enough that they're not giving me enough support, but they hid what they had.";
		mes "How can they do this to me?";
		completequest 16651;
		setquest 16652;
		close3;
	}
	if(isbegin_quest(16652) == 1){
		cloaknpc("Bagot#ep19re1",false,getcharid(0));
		cutin "ep18_bagot_01.png",2;
		mes "[Bagot]";
		mes "Oh, what's happening? It's noisy. Why are you making such a loud noise, Juncea?";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "Bagot, you...!";
		next;
		cutin "ep18_bagot_03.png",2;
		mes "[Bagot]";
		mes "What's wrong? Don't you have anything to say to me?";
		mes "Uh, you are...?";
		next;
		cutin "ep18_bagot_02.png",2;
		mes "[Bagot]";
		mes "What was his name again. I mean... weren't you the one born with the superior Rgan?";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "It's Senekiogand.";
		next;
		cutin "ep18_bagot_01.png",2;
		mes "[Bagot]";
		mes "Oh, I remember now. Come to think of it, Svegand was with me just a little while ago.";
		next;
		cutin "ep18_bagot_03.png",2;
		mes "[Bagot]";
		mes "He told me a lot of interesting stories. I had so much fun. These days young Rgans don't feel like one. Wahahaha!";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "Is that important now? I have something to say. It's important, Bagot.";
		next;
		cutin "ep18_bagot_03.png",2;
		mes "[Bagot]";
		mes "It just happens that I came here because I have something important to say to you, isn't it nice? It's good.";
		mes "Then we adults are going to have a talk, so can the child leave?";
		next;
		cutin "ep19_juncea03.png",2;
		mes "[Juncea]";
		mes "...Yes. Senekiogand. You stay out.";
		mes "It won't be a good story for a child.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "(I'd better do as I told. They'll be suspicious if I insist on staying.)";
		mes "Okay.";
		next;
		cutin "ep18_bagot_03.png",2;
		mes "[Bagot]";
		mes "I want to move somewhere.";
		mes "Should we go there, Juncea?";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "(Bagot looks a little off to me. Let's return to the Ice Castle and discuss it with <NAVI>[Lazy]<INFO>icas_in,245,197,0,101,0</INFO></NAVI>.)";
		cloaknpc("Bagot#ep19re1",true,getcharid(0));
		cloaknpc("Juncea#ep19re1",true,getcharid(0));
		completequest 16652;
		setquest 16653;
		close3;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16646) == 1";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16648) == 1 && countitem(1000846) > 0";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16649) == 1 && countitem(1000707) >= 10 || countitem(1000846) > 0";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16650) == 1 && countitem(1000707) >= 10 || countitem(1000846) > 0";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16651) == 1";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16652) == 1";
end;
}

jor_nest,34,139,3	duplicate(dummynpc2)	Bagot#ep19re1	4_EP18_BAGOT

jor_nest,26,142,3	script	Bagot#e19ms71	4_EP18_BAGOT,{
	if(!191_scroll_check()){
		cutin "ep18_bagot_02.png",1;
		mes "[Bagot]";
		mes "What? An experiment? Doesn't matter. You're in the way, leave.";
		close2;
		cutin "",255;
		warp "jor_back3",65,321;
		end;
	}
	if(isbegin_quest(17630) == 1){
		cutin "ep18_bagot_01.png",2;
		mes "[Bagot]";
		mes "Juncea, it's been a while. Did you forget how to smile already?";
		next;
		cutin "ep19_juncea04.png",0;
		mes "[Juncea]";
		mes "What? You're here. It's been a while I thought you wouldn't come back anymore.";
		next;
		cutin "ep18_bagot_01.png",2;
		mes "[Bagot]";
		mes "Am I not allowed to be here? I'm only telling you this because you seem to have forgotten, but isn't this a public laboratory?";
		next;
		cutin "ep19_juncea04.png",0;
		mes "[Juncea]";
		mes "You're already doing experiments on your own laboratory over there.";
		npctalk "It's like living in two houses, That's what it is. That place is a security facility.","Bagot#e19ms71",bc_self;
		next;
		cutin "ep18_bagot_01.png",2;
		mes "[Bagot]";
		mes "There is a senior Rgan who was born today.";
		mes "Did you already do the mana test for that Rgan? It was born in the lower levels. Check this superior-grade also and share the data with me~";
		next;
		cutin "ep19_juncea01.png",0;
		mes "[Juncea]";
		mes "Okay. I'm observing something now, so later.";
		next;
		cutin "ep18_bagot_02.png",2;
		mes "[Bagot]";
		mes "Do it now. I'm in a hurry.";
		next;
		cutin "ep19_juncea04.png",0;
		mes "[Juncea]";
		mes "I said, I'm observing something.";
		next;
		cutin "ep18_bagot_01.png",2;
		mes "[Bagot]";
		mes "You know, I came here to talk about something... well... anyway, I brought the Rgan.";
		mes "There's been some adjustments in the order of things.";
		next;
		cutin "ep19_juncea04.png",0;
		mes "[Juncea]";
		mes "Don't change the subject and get to the point.";
		next;
		cutin "ep18_bagot_01.png",2;
		mes "[Bagot]";
		mes "What I mean is that the priorities of research have changed. My research is now top priority. The most important.";
		next;
		cutin "ep18_bagot_03.png",2;
		mes "[Bagot]";
		mes "All the researchers, except Juncea? Is supporting my research first.";
		mes "The same goes for the data. I'm saying is you need to hurry up with the check and deliver it to me, do you understand?";
		next;
		cutin "ep19_juncea04.png",0;
		mes "[Juncea]";
		mes "I know what you mean. But why do I have to do that. I came here and worked on my research before you.";
		next;
		cutin "ep18_bagot_03.png",2;
		mes "[Bagot]";
		mes "The leader of this place liked my research and told me that they'll fully support it.";
		mes "Look. Rgan. I told you did I? That this was going to be her reaction.";
		next;
		cutin "",255;
		mes "[Sarekgand]";
		mes "You're correct. Tch- ";
		mes "As he said, the priorities of research have changed. Lasgand judged that his research was more urgent.";
		mes "All resources for other studies are to support his research as top priority.";
		next;
		cutin "ep18_bagot_03.png",2;
		mes "[Bagot]";
		mes "See? I won't say anything about Juncea's work personally, But you have to do everything what I want first.";
		next;
		cutin "ep19_juncea04.png",0;
		mes "[Juncea]";
		mes "This, this..! It doesn't make any sense? How could you do this to me? What about my research??";
		next;
		cutin "ep18_bagot_03.png",2;
		mes "[Bagot]";
		mes "I didn't mean to stop your research, but why are you so angry? Scary. Why my research is done, you can do whatever you want.";
		next;
		cutin "ep18_bagot_01.png",2;
		mes "[Bagot]";
		mes "Anyway, please take the data of these two and give them to me. I already have Lasgand's permission.";
		mes "Then, I'll go now!";
		completequest 17630;
		setquest 17631;
		questinfo_refresh;
		close2;
		cloaknpc("Bagot#e19ms71",true,getcharid(0));
		cloaknpc("Sarekgand#e19ms71",true,getcharid(0));
		cutin "",255;
		end;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17630) == 1";
end;
}

jor_nest,25,140,1	script	Sarekgand#e19ms71	4_EP19_RGAN_SR1,{
	if(isbegin_quest(17630) == 1)
		npctalk "Don't talk to me until I do. Haven't you learned that?","",bc_self;
	end;
}

jor_nest,24,142,3	duplicate(dummynpc2)	Lazy#e19ms72	4_EP19_RGAN_SR3

jor_nest,23,58,0	script	#e19ms81	HIDDEN_WARP_NPC,5,5,{
	end;
	
OnTouch:
	if(isbegin_quest(17628) == 1){
		cloaknpc("Rgan Priest#e19ms82",false,getcharid(0));
		cloaknpc("Horuru#e19ms81",false,getcharid(0));
		cloaknpc("Lehar#e19ms81",false,getcharid(0));
		cloaknpc("Rgan Bishop#e19ms81",false,getcharid(0));
	}
end;
}

jor_nest,23,58,5	script	Rgan Priest#e19ms81	4_EP19_RGAN_R1,{
	if(!191_scroll_check()){
		mes "[Rgan Priest]"; //TODO GET ORIGINAL DIALOGUE
		mes "Humans shouldn't be here. Get out of this place.";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	if(isbegin_quest(17628) == 1){
		mes "[Rgan Bishop]";
		mes "How can I manage this!!!";
		next;
		select("I...");
		mes "[Rgan Priest]";
		mes "Huh? Aren't you the newborn? Are you already familiar with the place already? You shouldn't come here carelessly.";
		next;
		select("Humans needs my eggshells.");
		mes "[Rgan Bishop]";
		mes "Why are you looking for your eggshells here? Weren't you born somewhere else?";
		npctalk "I can't go there because I have work here.","Rgan Priest#e19ms81",bc_self;
		next;
		mes "[Horuru]";
		mes "Let go! Let go of it!";
		next;
		select("What's going on?");
		mes "[Horuru]";
		mes "Oh, you might break the egg by mistake, how can you treat it like this!!";
		next;
		mes "[Rgan Bishop]";
		mes "Do you think all mistakes are the same? It doesn't matter if it'll wake up as a priest. Perhaps it can be a bishop too?";
		mes "Like the childrens who were born at the lower level today.";
		next;
		mes "[Lehar]";
		mes "Please forgive him. I'll pay attention to this guy. Please...";
		next;
		mes "[Rgan Priest]";
		mes "Huh, what are you going to do with this?";
		next;
		mes "[Rgan Bishop]";
		mes "... Well. I can't punish you too much on such a good day.";
		mes "You, what's your name?";
		next;
		mes "[Horuru]";
		mes "Ho, Horurururugand!";
		next;
		mes "[Lehar]";
		mes "I'm, I'm Leharjangand!";
		next;
		mes "[Rgan Bishop]";
		mes "I'll send you both down there. You'll be working there until I call you back.";
		mes "Leharjangand, You are responsible for teaching Horurugand a lesseon. I will dispose of you if you're not careful next time.";
		next;
		mes "[Rgan Priest]";
		mes "Oh!! Maybe Labor? Instead of disposing?";
		emotion ET_QUESTION,getnpcid(0,"Horuru#e19ms81");
		emotion ET_QUESTION,getnpcid(0,"Lehar#e19ms81");
		emotion ET_QUESTION,getcharid(3);
		next;
		emotion ET_QUESTION,getnpcid(0,"Horuru#e19ms81");
		emotion ET_QUESTION,getnpcid(0,"Lehar#e19ms81");
		emotion ET_QUESTION,getcharid(3);
		select("Is it something terrible?");
		mes "[Rgan Bishop]";
		mes "Hey, you. get him out of here!";
		next;
		cloaknpc("Rgan Priest#e19ms82",true,getcharid(0));
		cloaknpc("Horuru#e19ms81",true,getcharid(0));
		cloaknpc("Lehar#e19ms81",true,getcharid(0));
		mes "[Rgan Bishop]";
		mes "What's wrong with these workers these days. Tsk.";
		next;
		cloaknpc("Rgan Bishop#e19ms81",true,getcharid(0));
		mes "[Rgan Priest]";
		mes "Phew- yeah, child. What's going on?";
		next;
		select("My eggshells...");
		mes "[Rgan Priest]";
		mes "Your eggshell isn't here. Find your eggshells right away.";
		mes "Oh, by the way, what should I do with these broken eggs... poor things. I hope the other don't broke.";
		unittalk getcharid(3),strcharinfo(0) + " : " + "I should take one secretly.",bc_self;
		completequest 17628;
		setquest 17629;
		getitem 1000606,1;
		questinfo_refresh;
		close2;
		navigateto("jor_nest",22,141);
		end;
	}
	if(isbegin_quest(17629) == 1){
		mes "[Rgan Bishop]";
		mes "Don't you have work to do? Come to think of it, there are rumors of you going to the <NAVI>[human laboratory]<INFO>jor_nest,22,141,0,101,0</INFO></NAVI>. How was it? Is it really as terrifying as the rumors says?";
		close;
	}
	mes "[Rgan Priest]";
	mes "Are you familiar with the road? You have to be especially careful where there are eggs nearby. Or it'll break the one over here.";
	close;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17628) == 1";
end;
}

jor_nest,26,55,3	duplicate(dummynpc2)	Rgan Priest#e19ms82	4_EP19_RGAN_R2
jor_nest,21,55,7	duplicate(dummynpc2)	Horuru#e19ms81	21529
jor_nest,22,54,7	duplicate(dummynpc2)	Lehar#e19ms81	21529
jor_nest,25,57,3	duplicate(dummynpc2)	Rgan Bishop#e19ms81	4_EP19_RGAN_SR1

jor_nest,87,164,3	script	Lazy#e19ms71	4_EP19_RGAN_SR3,{
	if(isbegin_quest(17632) == 1){
		mes "[Lazy]";
		mes "Adventurer! Over here!!";
		next;
		select("Lazy!");
		mes "[Lazy]";
		mes "Phew- nervous? All, good? I thought I was caught.";
		next;
		mes "[Lazy]";
		mes "Aren't you very good, Adventurer? They really thought you're just a silly newborn Rgan.";
		next;
		mes "[Lazy]";
		mes "Did you get anything? What's happening with the Illusion?";
		mes "I couldn't find anything because I was held in Lasgand's room.";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "I don't have anything big. But I found out that the airship is here.";
		mes "Let's exchange information after going back to the Ice Castle instead of here.";
		mes "We need to check the airship...";
		next;
		mes "[Lazy]";
		mes "Did you find the location of the airship?";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "Yes. We have to stop by the lower level to rescue Horuru. He was taken away after breaking an egg.";
		next;
		mes "[Lazy]";
		mes "What! How did that happen!!! You should have told me as soon as we met!";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "They say it's the place where only low-rank Rgans can go to work, but they won't be attacked...";
		next;
		mes "[Lazy]";
		mes "Alone?";
		next;
		select("No, with Lehar.");
		mes "[Lazy]";
		mes "Oh, then we can relax. If they said they wouldn't attack their kin. Then nothing will happen immediately.";
		mes "Then, let's go check the airship first. Airship!";
		next;
		mes "[Lazy]";
		mes "Kyaa- It was once my dream to be an <NAVI>[airship]<INFO>jor_nest,24,252,0,101,0</INFO></NAVI> captain.";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "Your dream really changes a lot.";
		completequest 17632;
		setquest 17633;
		questinfo_refresh;
		close2;
		cloaknpc("Lazy#e19ms71",true,getcharid(0));
		end;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17632) == 1";
end;
}

jor_nest,24,252,0	script	#e19ms91	HIDDEN_WARP_NPC,5,5,{
	end;
	
OnTouch:
	if(isbegin_quest(17633) == 1)
		cloaknpc("Lazy#e19ms91",false,getcharid(0));
	if(isbegin_quest(17648) == 2){
		cloaknpc("Rgan Security#e19ms91",true,getcharid(0));
		cloaknpc("Rgan Security#e19ms92",true,getcharid(0));
		cloaknpc("Rgan Security#e19ms93",true,getcharid(0));
		cloaknpc("Rgan Security#e19ms94",true,getcharid(0));
	}
end;
}

jor_nest,24,252,1	script	Lazy#e19ms91	4_EP19_RGAN_SR3,{
	if(isbegin_quest(17633) == 1){
		mes "[Lazy]";
		mes "So this is where the Illusion's airship is?";
		next;
		select("There are quite a few guards.");
		mes "[Lazy]";
		mes "Still, there are no signs of it operating right now.";
		next;
		select("It looks like we can't do anything.");
		mes "[Lazy]";
		mes "We can't. Now that we know where it is, let's go back and prepare countermeasures. Now let's go to the <NAVI>[lower level]<INFO>jor_dun02,153,239,0,101,0</INFO></NAVI>.";
		completequest 17633;
		setquest 17634;
		close2;
		cloaknpc("Lazy#e19ms91",true,getcharid(0));
		navigateto("jor_dun02",153,239);
		end;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17633) == 1";
end;
}

jor_dun02,153,239,3	script	Rgan Security#e19ms95	4_EP19_RGAN_R1,{
	if(!191_scroll_check()){
		warp "jor_back3",65,321; //TODO
		end;
	}
	if(isbegin_quest(17634) == 1){
		mes "[Rgan Security]";
		mes "When will my turn end. Oh, It's been only 10 minutes since I began.";
		emotion ET_SLEEPY,getnpcid(0,"Rgan Security#e19ms95");
		next;
		cloaknpc("Lazy#e19ms92",false,getcharid(0));
		mes "[Lazy]";
		mes "Yo! Is everything alright?";
		next;
		mes "[Rgan Security]";
		mes "Oh! newborn... what brings you here...?";
		mes "This is not a place where an important figure like you can come recklessly.";
		mes "Did you do something wrong...?";
		next;
		mes "[Lazy]";
		mes "Of course not? I was born today and I'm learning the place.";
		next;
		mes "[Rgan Security]";
		mes "I don't think you'll ever come here again.";
		next;
		mes "[Lazy]";
		mes "And who said that?";
		next;
		mes "[Rgan Security]";
		mes "I'm sorry if it sounded rude. Let me explain.";
		next;
		mes "[Rgan Security]";
		mes "This is the place where workers does atonement labor, important people doesn't come here.";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "We came here because we've heard that there are a priest who have been sent to work here.";
		next;
		mes "[Rgan Security]";
		mes "Ah! There was! Please go in.";
		npctalk "Oh, Good job!","Lazy#e19ms92",bc_self;
		completequest 17634;
		setquest 17635;
		questinfo_refresh;
		close2;
		cloaknpc("Lazy#e19ms92",true,getcharid(0));
		cloaknpc("Miriam#e19ms91",false,getcharid(0));
		cloaknpc("Lehar#e19ms91",false,getcharid(0));
		cloaknpc("Horuru#e19ms91",false,getcharid(0));
		cloaknpc("Lazy#e19ms93",false,getcharid(0));
		end;
	}
	if(isbegin_quest(17635) == 1){
		mes "[Rgan Security]";
		mes "Have you finished your work?";
		close2;
		navigateto("jor_dun02",150,269);
		end;
	}
	mes "[Rgan Security]";
	mes "Have you resolved your business?";
	mes "Go take a look!";
	close;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17634) == 1";
end;
}

jor_dun02,150,237,7	duplicate(dummynpc2)	Lazy#e19ms92	4_EP19_RGAN_SR3

jor_dun02,134,257,3	script	Human#e19ms91	4_F_04,{
	npctalk "If you want to escape. We have escape by burning this place down.","Human#e19ms91",bc_self;
	end;
}

jor_dun02,136,254,3	script	Human#e19ms92	4_M_LGTPOOR,{
	npctalk "I don't know what I'm here for...","Human#e19ms92",bc_self;
	end;
}

jor_dun02,140,253,3	script	Human#e19ms93	4_M_02,{
	npctalk "I'll definitely finish what I have to do today.","Human#e19ms93",bc_self;
	end;
}

jor_dun02,146,255,1	script	Rgan#e19ms92	EP19_RGAN_C,{
	npctalk "Sle... pp... y...","Rgan#e19ms92",bc_self;
	end;
}

jor_dun02,147,258,3	script	Rgan#e19ms93	EP19_RGAN_C,{
	npctalk "Ti... red...","Rgan#e19ms93",bc_self;
	end;
}

jor_dun02,150,269,0	script	#e19ms94	HIDDEN_WARP_NPC,5,5,{
	end;
	
OnTouch:
	if(isbegin_quest(17635) == 1){
		cloaknpc("Miriam#e19ms91",false,getcharid(0));
		cloaknpc("Lehar#e19ms91",false,getcharid(0));
		cloaknpc("Horuru#e19ms91",false,getcharid(0));
		cloaknpc("Lazy#e19ms93",false,getcharid(0));
	}
end;
}

jor_dun02,150,269,3	script	Miriam#e19ms91	21529,{
	if(isbegin_quest(17635) == 1){
		mes "[Rgan]";
		mes "<FONT SIZE = 9>It's the bishop and the priest who were born today!</FONT>";
		next;
		select("Who are we looking at?");
		mes "[Rgan]";
		mes "<FONT SIZE = 9>They're here!</FONT>";
		next;
		mes "[Lazy]";
		mes "Just whisper, who are we looking at..? They all look the same.";
		next;
		mes "[Miriam]";
		mes "<FONT SIZE = 9>I'm Miriam. This is Lehar, and that's Horuru.</FONT>";
		npctalk "As expected adventurer! You're here to save us!","Lehar#e19ms91",bc_self;
		npctalk "I was very hopeful. Could it be that you were also brought here to work with us!","Horuru#e19ms91",bc_self;
		next;
		select("Everyone's safe. It's a relief.");
		mes "[Lazy]";
		mes "I only heard it from the adventurer and I thought it was terrifying.";
		mes "Hahaha. Our adventurer tends to exaggerate, a bit.";
		next;
		mes "[Miriam]";
		mes "It's terrifying. Look around. Besides lesser Rgans, there are also humans.";
		next;
		mes "[Lehar]";
		mes "I can name atleast twenty cases of labor violations.";
		cloaknpc("Human#e19ms94",false,getcharid(0));
		next;
		mes "[Human]";
		mes "Priest, may I take a break?";
		next;
		mes "[Miriam]";
		mes "Uh, sure. Get some rest.";
		next;
		mes "[Human]";
		mes "Thank you.";
		cloaknpc("Human#e19ms94",true,getcharid(0));
		next;
		select("A human? Whys is a human here?");
		mes "[Miriam]";
		mes "I think they're human who've been brought with the airship. They were abducted and forced to work.";
		next;
		mes "[Lehar]";
		mes "I think there are people who used to be experiments like Lunch, and there are also just normal workers.";
		next;
		mes "[Horuru]";
		mes "I often ran into a human on the way here.";
		unittalk getcharid(3),"You don't have to come here because unlike us, you're a superior-grade right?",bc_self;
		next;
		mes "[Lazy]";
		mes "I was set out to find paradise, but fell into the abyss.";
		next;
		select("Don't do this here, just go back.");
		npctalk "They're here to save us!","Lehar#e19ms91",bc_self;
		npctalk "It's really true!","Horuru#e19ms91",bc_self;
		mes "[Lehar]";
		mes "Can we leave?";
		next;
		mes "[Lazy]";
		mes "I already told the Rgan who was standing guard, so it should be fine. The bishop came and took them, who would complain?";
		next;
		mes "[Horuru]";
		mes "Well. That is true. Let's get out of here. I can't stand to stay for another hour here.";
		next;
		select("Will we leave the humans behind?");
		mes "[Miriam]";
		mes "There's nothing we can do right now. We have to deal with the Rgans to rescue them.";
		next;
		mes "[Lazy]";
		mes "We need to take a measure for that also. We have a lot of work to do after going back to the <NAVI>[Ice Castle]<INFO>icas_in,34,189,0,101,0</INFO></NAVI>.";
		completequest 17635;
		setquest 17636;
		close2;
		cloaknpc("Miriam#e19ms91",true,getcharid(0));
		cloaknpc("Lehar#e19ms91",true,getcharid(0));
		cloaknpc("Horuru#e19ms91",true,getcharid(0));
		cloaknpc("Lazy#e19ms93",true,getcharid(0));
		end;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17635) == 1";
end;
}


jor_dun02,145,271,5	script	Lehar#e19ms91	21529,{
	npctalk "I believed in you adventurer!","Lehar#e19ms91",bc_self;
	end;
}

jor_dun02,141,267,5	script	Horuru#e19ms91	21529,{
	npctalk "I was terrible a while ago, I need to stop being hot-tempered...","Horuru#e19ms91",bc_self;
	end;
}

jor_dun02,145,267,7	script	Lazy#e19ms93	4_EP19_RGAN_SR3,{
	npctalk "Is this the place?","Lazy#e19ms93",bc_self;
	end;
}

jor_dun02,148,264,7	duplicate(dummynpc2)	Human#e19ms94	4_M_ORIENT02


jor_nest,124,204,0	script	#hw_ep19re1	HIDDEN_WARP_NPC,5,5,{
	end;
	
OnTouch:
	if(isbegin_quest(17638) == 1 || isbegin_quest(16645) == 1)
		cloaknpc("Lazy#ep19re1",false,getcharid(0));
end;
}

jor_nest,127,207,3	script	Lazy#ep19re1	4_EP19_RGAN_SR3,{
	if(!191_scroll_check()){
		mes "[Lazy]";
		mes "Huh, I already transformed myself properly here. Where is your transformation scroll? Go and get it, adventurer!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	if(isbegin_quest(17638) == 1){
		mes "[Lazy]";
		mes "Hm, I can't see the humans. The number of high-ranking Rgans also decreased.";
		mes "Where is everyone? Did they go inside? Or are they gone?";
		next;
		mes "[Lazy]";
		mes "Anyway, this is good. Let's sneak around and get some information.";
		mes "Since we are a just an adorable newborn senior and superior Rgans, they will think we're cute even if we're caught walking around.";
		next;
		mes "[Lazy]";
		mes "But there are areas where a senior Rgan can't walk around freely, so I'll go in those deep places.";
		next;
		mes "[Lazy]";
		mes "Adventurer, why don't you go to the laboratory of Juncea and Bagot? I think there are some Rgans guarding nearby, so it would be beast to sneak around too.";
		next;
		select("I'll do that.");
		mes "[Lazy]";
		mes "Let's pretend to be the cutest newborn Rgan who doesn't know anything.";
		next;
		mes "[Lazy]";
		mes "Let's meet up here after the investigation, adventurer.";
		mes "I'll be back after a while. *Cries*!";
		completequest 17638;
		setquest 16636;
		setquest 16637;
		setquest 16638;
		questinfo_refresh;
		close2;
		cloaknpc("Lazy#ep19re1",true,getcharid(0));
		end;
	}
	if(isbegin_quest(16645) == 1){
		mes "[Lazy]";
		mes "Have you looked around the places that we've talked about? I've pretty much seen everything.";
		next;
		select("I also did.");
		mes "[Lazy]";
		mes "Then adventurer, let's quickly exchange information and set a course of action for the future.";
		mes "From the Illusions residences, did you get anything?";
		next;
		if(select("Relationship between the two Illusions:The experiment that the Illusion are doing") == 1){
			mes "[Lazy]";
			mes "The relationship between the two Illusions? How does it look?";
			mes "Ho, you think Juncea is being ignored compared to Bagot?";
			next;
			mes "[Lazy]";
			mes "She's getting lesser research supplies, and Lasgand lost interest. We've seen that together.";
			mes "Did you bring the ledger? You're so talented. Let' see.";
			next;
			mes "[Lazy]";
			mes "No, is there really such difference? I haven't thought that it would be like this.";
			mes "What kind of research are these two doing that makes such a difference? I'm dying to know. Did you find anything?";
			next;
			mes "[Lazy]";
			mes "You got it? I'm so proud of you, as expected from our adventurer. Hurry and tell me.";
			mes "Well, it looks similar from the outside. But to you, Bagot's research seems more dangerous.";
			next;
		}
		mes "[Lazy]";
		mes "Do you think it's similar to the False God that you saw in Rachel? What is he planning to do with it. That's very suspicious!";
		next;	
		mes "[Lazy]";
		mes "With this... I thought the two Illusions were in not in a bad relationship, but it's much worse than I thought, I think we can use this. Don't you think?";
		next;
		mes "[Lazy]";
		mes "You did a fantastic job, adventurer. Not good as mine, but quite keen. It's not enough to earn a living from my work, but it's enough to buy snacks.";
		next;
		mes "[Lazy]";
		mes "The Illusion scientist that came first... What has her name again? Yeah, from Juncea's point of view, it'll feel like pulling a boulder then getting a rock.";
		next;
		mes "[Lazy]";
		mes "And from Bagot's point of view, knowing Juncea's thoughts wouldn't be a pity.";
		mes "If we gently deal with them, and drive them apart~ We might be able to deal with them one by one.";
		next;
		mes "[Lazy]";
		mes "Say, do you think it's a good idea? Adventurer. What do you think about of my opinion?";
		next;
		if(select("Say that it's a good idea.:Say that it's not good.") == 2){
			mes "[Lazy]";
			mes "Not good? Well, what do you think adventurer? Do we have an alternative?";
			next;
			mes "[Lazy]";
			mes "You have to give me an alternative if you object. Adventurer, if there's no alternative, then let's give it a try.";
			next;
			select("I can't help it, so yes.");
		}
		mes "[Lazy]";
		mes "Great. Then, it's decided?";
		mes "In order to drive the two Illusion, we have to get closer with them first.";
		next;
		mes "[Lazy]";
		mes "Juncea seems to like you adventurer, so you'll handle Juncea.";
		mes "The other guy, said that I looked weird. He said that even though he is the weird one.";
		next;
		mes "[Lazy]";
		mes "Let's poison their mind as we gain their favor.";
		mes "Shall we go? Can you do it?";
		next;
		select("Nod in agreement.");
		mes "[Lazy]";
		mes "Oh, trust me. Our adventurer, is very reliable.";
		mes "Then, let's meet at the Ice Castle next time we meet. They might notice us if we keep whispering like this.";
		next;
		mes "[Lazy]";
		mes "I'll go to Bagot. Then, shall we proceed with out own things?";
		mes "Go to <NAVI>[Juncea]<INFO>jor_nest,31,140,0,101,0</INFO></NAVI>. Goodluck, Adventurer.";
		completequest 16645;
		setquest 16646;
		questinfo_refresh;
		close2;
		cloaknpc("Lazy#ep19re1",true,getcharid(0));
		end;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17638) == 1";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16645) == 1";
end;
}

function	script	191_re_check	{
	return	(isbegin_quest(16640) == 2 && isbegin_quest(16642) == 2 && isbegin_quest(16643) == 2 && isbegin_quest(16644) == 2);
}

jor_nest,51,152,0	script	#hw_ep19re2	HIDDEN_WARP_NPC,5,5,{
	end;
	
OnTouch:
	if(isbegin_quest(16636) == 1){
		unittalk getcharid(3),strcharinfo(0) + " : Let's go a little further inside.",bc_self;
		completequest 16636;
		setquest 16639;
		setquest 16640;
		questinfo_refresh;
	}
	if(isbegin_quest(16639) == 1)
		cloaknpc("Pile of Papers#ep19re1",false,getcharid(0));
	if(isbegin_quest(16339) == 2 && isbegin_quest(16340) == 1)
		cloaknpc("Pile of Papers#ep19re2",false,getcharid(0));
	if(isbegin_quest(16646) == 1 || isbegin_quest(16648) == 1 || isbegin_quest(16649) == 1 || isbegin_quest(16650) == 1 || isbegin_quest(16651) == 1 || isbegin_quest(16652) == 1)
		cloaknpc("Juncea#ep19re1",false,getcharid(0));
	if(isbegin_quest(16655) == 1)
		cloaknpc("Unnoticed Box#ep19re1",false,getcharid(0));
	if(isbegin_quest(16656) == 1 || isbegin_quest(16657) == 1){
		cloaknpc("Unnoticed Box#ep19re1",false,getcharid(0));
		cloaknpc("Empty Test Tube#ep19re1",false,getcharid(0));
	}
	if(isbegin_quest(16658) == 1)
		cloaknpc("Rgan#ep19re1",false,getcharid(0));
	if(isbegin_quest(17625) == 1 || isbegin_quest(17629) == 1 || isbegin_quest(17631) == 1)
		cloaknpc("Juncea#e19ms71",false,getcharid(0));
	if(isbegin_quest(17631) == 1)
		cloaknpc("Juncea#e19ms71",false,getcharid(0));
	if(isbegin_quest(17632) == 1)
		cloaknpc("Lazy#e19ms71",false,getcharid(0));
end;
}

jor_nest,26,175,0	duplicate(#hw_ep19re2)	#hw_ep19re2-1	HIDDEN_WARP_NPC,5,5
jor_nest,43,141,0	duplicate(#hw_ep19re2)	#hw_ep19re2-2	HIDDEN_WARP_NPC,5,5

jor_nest,19,142,3	script	Pile of Papers#ep19re1	4_EP18_PAPERS,{
	if(isbegin_quest(16639) == 1){
		mes "[" + strcharinfo(0) + "]";
		mes "The pile of papers are piled in an unorganized way.";
		mes "I may take some time to read and understand.";
		next;
		progressbar "FFFF00",5;
		mes "[Report]";
		mes "...Inject artifial mana into an object...";
		mes "...Using this method, results show that we could increase the grade of the unhatched eggs...";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "Is this the only information I can find...";
		completequest 16639;
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "(Let's take a closer look inside the laboratory.)";
		cloaknpc("Pile of Papers#ep19re2",false,getcharid(0));
		close;
	}
	if(isbegin_quest(16639) == 2){
		mes "[" + strcharinfo(0) + "]";
		mes "I've already examined this pile of papers.";
		close;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16636) > 0 && isbegin_quest(16639) < 2";
end;
}

jor_nest,21,195,3	script	Pile of Papers#ep19re2	4_EP18_PAPERS,{
	if(isbegin_quest(16640) == 1){
		mes "[" + strcharinfo(0) + "]";
		mes "The pile of papers are piled in an unorganized way.";
		mes "I may take some time to read and understand.";
		next;
		progressbar "FFFF00",5;
		mes "[Report]";
		mes "...In addition to that, using the same method for object that have already been born...";
		mes "...and the fact that they can be reborn as something better...";
		next;
		cloaknpc("Yugurungand#ep19re2",false,getcharid(0));
		npctalk "Who's there, an intruder!","Yugurungand#ep19re2",bc_self;
		mes "[Yugurungand]";
		mes "Didn't the outsiders left? What are you doing here!";
		next;
		mes "[Yugurungand]";
		mes "Hm? I've never seen you before. A priest-grade?";
		mes "Oh, I see. You're the new born. That's cute. What's your name?";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "Senekiogand.";
		next;
		mes "[Yugurungand]";
		mes "Okay, Senekiogand. Did you come all the way here after wandering alone?";
		mes "There are many dangerous things here in the residence of the outsiders. You shouldn't come here recklessly.";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "(...It's convenient that I don't need to make excuses.)";
		mes "Yes, I understand. I won't come here again.";
		next;
		mes "[Yugurungand]";
		mes "Good, you're nice. My name is Yugurungand. I'm the one in charge of guarding the humans from the outside.";
		next;
		mes "[Yugurungand]";
		mes "It's for their protection, then... there's also the purpose of monitoring. It's because humans can't be trusted.";
		next;
		mes "[Yugurungand]";
		mes "But, those humans doesn't seem to trust each other.";
		next;
		mes "[Yugurungand]";
		mes "They used to pretend to be close to each other, but recently, they've shown their true nature";
		next;
		mes "[Yugurungand]";
		mes "The person who mainly uses this place... Yes, Juncea's place seems to have been taken by Bagot.";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "Is she going to lose this place?";
		next;
		mes "[Yugurungand]";
		mes "Yes. Bagot has already taken all of Lasgand's favor.";
		next;
		mes "[Yugurungand]";
		mes "It seems that they're trying to get results... But it's too much right now.";
		mes "Oh, did I tell you a story that's too hard to understand?";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "(It's better to pretend that I didn't understand)";
		mes "It's okay. It was fun.";
		next;
		mes "[Yugurungand]";
		mes "Haha, you child. Your expression completely shows that you didn't understand anything.";
		mes "Don't worry about it, I just talked too much.";
		next;
		mes "[Yugurungand]";
		mes "I'll return first, Senekiogand, you should return quickly without touching anything around here.";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "Don't worry. Thank you, Yugurungand.";
		next;
		cloaknpc("Yugurungand#ep19re2",true,getcharid(0));
		mes "[" + strcharinfo(0) + "]";
		mes "(I think I've figured out everything I can about Juncea's lab.)";
		completequest 16640;
		next;
		if(!191_re_check()){
			mes "[" + strcharinfo(0) + "]";
			mes "(Let's go somewhere else.)";
		} else {
			mes "[" + strcharinfo(0) + "]";
			mes "(I've already looked around enough at other places... let's go back to <NAVI>[Lazy]<INFO>jor_nest,127,207,0,101,0</INFO></NAVI>.)";
			setquest 16645;
			questinfo_refresh;
		}
		close;
	}
	if(isbegin_quest(16640) == 2){
		mes "[" + strcharinfo(0) + "]";
		mes "I've already examined this pile of papers.";
		close;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16636) > 0 && isbegin_quest(16640) < 2";
end;
}

jor_nest,22,192,3	duplicate(dummynpc2)	Yugurungand#ep19re2	4_EP19_RGAN_SR1

jor_nest,177,176,0	script	#hw_ep19re4	HIDDEN_WARP_NPC,5,5,{
	end;
	
OnTouch:
	if(isbegin_quest(16638) == 1){
		completequest 16638;
		setquest 16643;
		setquest 16644;
		questinfo_refresh;
	}
	if(isbegin_quest(16643) == 1)
		cloaknpc("Pile of Papers#ep19re5",false,getcharid(0));
	if(isbegin_quest(16644) == 1){
		cloaknpc("Rgan Guard#ep19re1",false,getcharid(0));
		cloaknpc("Rgan Guard#ep19re2",false,getcharid(0));
	}
end;
}

jor_nest,181,173,3	script	Pile of Papers#ep19re5	4_EP18_PAPERS,{
	if(isbegin_quest(16643) == 1){
		mes "[" + strcharinfo(0) + "]";
		mes "There's a thing that appears to be a ledger. Let's take a look...";
		mes "There's a lot of content, it'll take time to figure everything what I need.";
		next;
		progressbar "FFFF00",5;
		mes "[" + strcharinfo(0) + "]";
		mes "It's written here that a large amount of supplies flowed into Bagot's laboratory.";
		mes "Compared to that, the amount allocated to Juncea is significantly smaller.";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "I can use this, I think? Let's take a look...";
		next;
		mes "[" + strcharinfo(0) + "]";
		getitem 1000845,1;
		completequest 16643;
		mes "(I took the accounting ledger.)";
		if(191_re_check()){
			next;
			mes "[" + strcharinfo(0) + "]";
			mes "(I've already looked around enough at other places... let's go back to <NAVI>[Lazy]<INFO>jor_nest,127,207,0,101,0</INFO></NAVI>.)";
			setquest 16645;
			questinfo_refresh;
		}
		close2;
		cloaknpc("Pile of Papers#ep19re5",true,getcharid(0));
		end;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16638) > 0 && isbegin_quest(16643) < 2";
end;
}

jor_nest,173,177,3	script	Rgan Guard#ep19re1	4_EP19_RGAN_R2,{
	if(isbegin_quest(16644) == 1){
		mes "[" + strcharinfo(0) + "]";
		mes "(I'll pretend to be a Rgan who's just passing by, Let's casually hang around next to then.... Fortunately, they don't mind talking amongst themselves.)";
		next;
		mes "[Rgan Guards Captain]";
		mes "Oh~ I got scolded by Yugurungand again. It's hard to be a guard, really.";
		next;
		mes "[Rgan Guards Member]";
		mes "It must have been a big deal! Something must have happened!";
		next;
		mes "[Rgan Guards Captain]";
		mes "No, it's not something about you members... it's about Lasgand's residence, there's too little personnel there.";
		next;
		mes "[Rgan Guards Captain]";
		mes "The human with black hair just come and go there recently. We are too relaxed about this. Hm.";
		next;
		mes "[Rgan Guards Member]";
		mes "You mean the guy named Bagot! He's been visiting Lasgand a lot lately!";
		next;
		mes "[Rgan Guards Captain]";
		mes "Uh, yeah. I don't know if we can trust him.";
		next;
		mes "[Rgan Guards Captain]";
		mes "You seem to trust him, but... that's not the case for Yugurungand.";
		next;
		mes "[Rgan Guards Member]";
		mes "Really? Then, what should we do?";
		next;
		mes "[Rgan Guards Captain]";
		mes "That's the problem. Right now, we can only be careful.";
		mes "We're going to reallocate the personnel, so be familiar with your new places. Let's get back to work again! Go!";
		next;
		mes "[Rgan Guards Member]";
		mes "Yes sir!";
		next;
		cloaknpc("Rgan Guard#ep19re1",true,getcharid(0));
		cloaknpc("Rgan Guard#ep19re2",true,getcharid(0));
		mes "[" + strcharinfo(0) + "]";
		mes "(I've everything they talked about.)";
		completequest 16644;
		next;
		if(!191_re_check()){
			mes "[" + strcharinfo(0) + "]";
			mes "(Let's go somewhere else.)";
		} else {
			mes "[" + strcharinfo(0) + "]";
			mes "(I've already looked around enough at other places... let's go back to <NAVI>[Lazy]<INFO>jor_nest,127,207,0,101,0</INFO></NAVI>.)";
			setquest 16645;
			questinfo_refresh;
		}
		close;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16638) > 0 && isbegin_quest(16644) < 2";
end;
}

jor_nest,168,177,5	duplicate(Rgan Guard#ep19re1)	Rgan Guard#ep19re2	4_EP19_RGAN_R3


jor_nest,65,259,0	script	#hw_ep19re6	HIDDEN_WARP_NPC,5,5,{
	end;
	
OnTouch:
	if(isbegin_quest(16637) > 0)
		cloaknpc("#warp_ep19re1",false,getcharid(0));
end;
}

jor_nest,66,260,3	script	#warp_ep19re1	HIDDEN_NPC,2,2,{
	end;
	
OnTouch:
	if(isbegin_quest(16637) == 0) end;
	if(isbegin_quest(16637) == 1){
		completequest 16637;	
		setquest 16641;
		setquest 16642;
		warp "jor_dun03",58,52;
		end;
	}
	if(isbegin_quest(16645) < 1){
		warp "jor_dun03",58,52;
		end;
	}
	if(isbegin_quest(16659) == 1 || isbegin_quest(16660) == 1){
		warp "jor_dun03",58,52;
		end;
	}
	if(isbegin_quest(17649) == 2 && checkquest(16663,PLAYTIME) == -1 || checkquest(16663,PLAYTIME) == 2)
		cloaknpc("Aroron#ep19re2",false,getcharid(0));
end;

OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16637) == 1";
end;
}

jor_dun03,58,54,0	script	#hw_ep19re7	HIDDEN_WARP_NPC,5,5,{
	end;
	
OnTouch:
	if(isbegin_quest(16641) == 1)
		cloaknpc("Pile of Papers#ep19re3",false,getcharid(0));
	if(isbegin_quest(16641) == 2 && isbegin_quest(16642) == 1)
		cloaknpc("Pile of Papers#ep19re4",false,getcharid(0));
	if(isbegin_quest(16659) == 1 || isbegin_quest(16660) == 1)
		cloaknpc("Juncea#ep19re2",false,getcharid(0));
end;
}

jor_dun03,74,71,3	script	Pile of Papers#ep19re3	4_EP18_PAPERS,{
	if(isbegin_quest(16641) == 1){
		mes "[" + strcharinfo(0) + "]";
		mes "There are piles of papers that are written in sleek handwriting.";
		mes "It will take some time to read and sort out the necessary contents.";
		next;
		progressbar "FFFF00",5;
		mes "[Report]";
		mes "...To the individuals who are already born using the mana of the snake...";
		mes "...It can create better and stronger objects...";
		unittalk getcharid(3),strcharinfo(0) + " : Is this similar to what Juncea's doing? I'm not sure....",bc_self;
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "When I look around at the laboratory's experiments, It feels strange somehow....";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "That's right. It feels like the False God that I met in Rachel...";
		mes "I feel a similar force from it.";
		completequest 16641;
		cloaknpc("Pile of Papers#ep19re4",false,getcharid(0));
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "(Let's take a closer look inside the laboratory.)";
		close;
	}
	if(isbegin_quest(16641) == 2){
		mes "[" + strcharinfo(0) + "]";
		mes "I've already examined this pile of papers.";
		close;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16637) > 0 && isbegin_quest(16641) < 2";
end;
}

jor_dun03,49,80,3	script	Pile of Papers#ep19re4	4_EP18_PAPERS,{
	if(isbegin_quest(16642) == 1){
		mes "[" + strcharinfo(0) + "]";
		mes "There are piles of papers that are written in sleek handwriting.";
		mes "That document in the gap..., it says top secret. Let's read it carefully.";
		next;
		progressbar "FFFF00",5;
		mes "[Top Secret]";
		mes "...Like the experiment in Rachel, the specimen that has started mutation undergoes biological replacement at the cellular level...";
		next;
		mes "[Top Secret]";
		mes "It's impossible to return the specimen to its original shape once it starts mutating.";
		next;
		mes "[Top Secret]";
		mes "...However, even if the mana wavelength of the experiment is correct, the possibility of reversibility remains low, but this is all just a hypothesis...";
		next;
		mes "[Top Secret]";
		mes "...In order to reverse it, we'll need the right materials with the proper mana wavelength and someone who can inject the correct mana...";
		next;
		mes "[Top Secret]";
		mes "But, the importance of this research is low since there's no need to reutn the specimens to it's original form...";
		next;
		mes "[Top Secret]";
		mes "It may have a big impact if it was leaked to the Rgans, so I'll classify this as top secret....";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "(Juncea said that they don't do experiments on high-grade Rgans... anyway...)";
		completequest 16642;
		next;
		if(!191_re_check()){
			mes "[" + strcharinfo(0) + "]";
			mes "(Let's go somewhere else.)";
		} else {
			mes "[" + strcharinfo(0) + "]";
			mes "(I've already looked around enough at other places... let's go back to <NAVI>[Lazy]<INFO>jor_nest,127,207,0,101,0</INFO></NAVI>.)";
			setquest 16645;
		}
		close;
	}
	if(isbegin_quest(16642) == 2){
		mes "[" + strcharinfo(0) + "]";
		mes "I've already examined this pile of papers.";
		close;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16637) > 0 && isbegin_quest(16642) < 2";
end;
}

icas_in,253,171,0	script	#ep19_evt07	HIDDEN_WARP_NPC,2,2,{
	end;
	
OnTouch:
	if(isbegin_quest(16647) == 1 || isbegin_quest(16648) == 1 || isbegin_quest(16649) == 1 || isbegin_quest(16650) == 1){
		cloaknpc("Wirorong#ep19re1",false,getcharid(0));
		cloaknpc("Jaerorong#ep19re2",false,getcharid(0));
	}
	if(isbegin_quest(18136) == 1){
		cloaknpc("Maram#ep19maram05",false,getcharid(0));
		cloaknpc("Miriam#ep19miriam06",false,getcharid(0));
		cloaknpc("Rescued Survivor#ep19_01",false,getcharid(0));
		cloaknpc("Rescued Survivor#ep19_02",false,getcharid(0));
	}
end;
}

icas_in,248,197,3	script	Wirorong#ep19re1	4_EP19_IWIN,{
	if(isbegin_quest(16647) == 1){
		cutin "ep19_iwin01.png",2;
		mes "[Wirorong]";
		mes "*crunch*, *crunch*...";
		mes "Why are you looking at me like that? Are you interested in the Iwin's secret ice cookie?";
		next;
		cutin "",255;
		if(select("Ask for one.:I'm not interested.") == 2){
			cutin "ep19_iwin01.png",2;
			mes "[Wirorong]";
			mes "Well, you're featherless and you can't hide your expressions .";
			mes "It's not a heartless expression. Whatever.";
			close3;
		}
		//TODO Wirorong Dialogue for 16674
		cutin "ep19_iwin01.png",2;
		mes "[Wirorong]";
		mes "Our Iwin Patissier is really good!";
		next;
		mes "[Wirorong]";
		mes "Here, don't hesitate to try it.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "(I'll just say that I receive it from the Iwins who were walking around.... Let's bring it to <NAVI>[Juncea]<INFO>jor_nest,31,140,0,101,0</INFO></NAVI>.)";
		getitem 1000846,1;
		completequest 16647;
		setquest 16648;
		close3;
	}
	if(isbegin_quest(16648) == 1 || isbegin_quest(16649) == 1 || isbegin_quest(16650) == 1){
		if(countitem(1000846) > 0){
			cutin "ep19_iwin01.png",2;
			mes "[Wirorong]";
			mes "Don't you already have one? You want more? No.";
			next;
			mes "[Wirorong]";
			mes "It will freeze if you eat too much at once.";
			mes "It's really delicious, but it's very cold. Eat it one at a time.";
			close3;
		}
		mes "[Wirorong]";
		mes "*crunch*, *crunch*...";
		mes "Are you going to keep looking at me?";
		next;
		if(select("Ask for one more.:Avoid your gaze.") == 2){
			cutin "ep19_iwin01.png",2;
			mes "[Wirorong]";
			mes "What a shame. You don't have feathers and no shame.";
			close3;
		}
		mes "[Wirorong]";
		mes "*crunch*,*crunch*,*crunch*!";
		mes "Yeah. You want another one, right?";
		getitem 1000846,1;
		close3;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16647) == 1";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16648) == 1 && countitem(1000846) == 0";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16649) == 1 && countitem(1000846) == 0";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16650) == 1 && countitem(1000846) == 0";
end;
}

icas_in,245,197,7	script	Jaerorong#ep19re2	4_EP19_IWIN,{
	if(isbegin_quest(16647) == 1){
		cutin "ep19_iwin01.png",2;
		mes "[Jaerorong]";
		mes "*crunch*, *crunch*! Oh, It's cold. But it's delicious.";
		next;
		mes "[Jaerorong]";
		mes "Oh, what I'm eating? Ice Cookies. It's delicious. Do you want one?";
		next;
		cutin "",255;
		if(select("Ask for one.:No thanks.") == 2){
			cutin "ep19_iwin01.png",2;
			mes "[Jaerorong]";
			mes "Oh, is it too cold? Nothing we can do then.";
			close3;
		}
		cutin "ep19_iwin01.png",2;
		mes "[Jaerorong]";
		mes "Our Iwin Patissier is really good!";
		next;
		mes "[Jaerorong]";
		mes "Here, don't hesitate to try it.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "(I'll just say that I receive it from the Iwins who were walking around.... Let's bring it to <NAVI>[Juncea]<INFO>jor_nest,31,140,0,101,0</INFO></NAVI>.)";
		getitem 1000846,1;
		completequest 16647;
		setquest 16648;
		close3;
	}
	if(isbegin_quest(16648) == 1 || isbegin_quest(16649) == 1 || isbegin_quest(16650) == 1){
		if(countitem(1000846) > 0){
			cutin "ep19_iwin01.png",2;
			mes "[Jaerorong]";
			mes "Don't you already have one? You want more? No.";
			mes "Your throat will freeze if you eat too much.";
			close3;
		}
		cutin "ep19_iwin01.png",2;
		mes "[Jaerorong]";
		mes "*crunch*,*crunch*! Haha, it's so delicious.";
		next;
		mes "[Jaerorong]";
		mes "Oh, hello there. Do you want some more Ice Cookies?";
		next;
		if(select("Ask for one more.:No thanks.") == 2){
			cutin "ep19_iwin01.png",2;
			mes "[Jaerorong]";
			mes "Well, your throat will freeze if you eat too much.";
			close3;
		}
		//TODO Jaerorong extra cookie dialogue
		mes "[Jaerorong]";
		mes "*crunch*,*crunch*,*crunch*!";
		mes "Yeah. You want another one, right?";
		getitem 1000846,1;
		close3;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16647) == 1";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16648) == 1 && countitem(1000846) == 0";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16649) == 1 && countitem(1000846) == 0";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16650) == 1 && countitem(1000846) == 0";
end;
}

icas_in,255,200,0	script	#hw_ep19re5	HIDDEN_WARP_NPC,2,2,{
	end;
	
OnTouch:
	if(isbegin_quest(16653) == 1 || isbegin_quest(16654) == 1){
		cloaknpc("Lazy#ep19re2",false,getcharid(0));
		cloaknpc("Miriam#ep19re1",false,getcharid(0));
	}
end;
}

icas_in,245,197,7	script	Lazy#ep19re2	4_EP19_LAZY,{
	if(isbegin_quest(16653) == 1){
		cutin "ep19_leizi01.png",2;
		mes "[Lazy]";
		mes "Hey, the job went well. How is it on your side?";
		mes "I'm resting for a while. I'm going to look at everything bit by bit.";
		next;
		mes "[Lazy]";
		mes "I also did some work with Bagot. He's been talking about a lot of suspicious things about Juncea.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "I've gotten closer to Juncea too.";
		mes "I also informed her that Bagot.";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "Juncea doesn't seem to know that Bagot is receiving more supplies than she is.";
		next;
		cutin "ep19_leizi01.png",2;
		mes "[Lazy]";
		mes "Really? Then she might aim for revenge.";
		mes "Now is the time to convince Juncea....";
		next;
		cutin "ep18_miriam_01.png",2;
		mes "[Miriam]";
		mes "Both of you, how is it going?";
		mes "I heard what Lazy's story, but... how about you adventurer?";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "Well, I also feel like I need to get things done quickly.";
		mes "Bagot came to Juncea earlier.";
		next;
		cutin "ep19_leizi03.png",2;
		mes "[Lazy]";
		mes "What! Really? You allowed them to talk, adventurer?";
		mes "We can't let that happen!";
		next;
		cutin "ep18_miriam_01.png",2;
		mes "[Miriam]";
		mes "But there wasn't a way stop it, right?";
		mes "Since you were pretending to be a young Rgan, it would have been difficult to step in.";
		next;
		cutin "ep19_leizi01.png",2;
		mes "Lazy";
		mes "But <NAVI>[Miriam]<INFO>icas_in,249,199,0,101,0</INFO></NAVI>, didn't you overheard what we were talking about?";
		completequest 16653;
		setquest 16654;
		questinfo_refresh;
		close3;
	}
	if(isbegin_quest(16654) == 1){
		mes "[Lazy]";
		mes "Ouch, my body.";
		mes "Busy, so busy.";
		close;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16653) == 1";
end;
}

icas_in,249,199,3	script	Miriam#ep19re1	4_EP18_MIRIAM,{
	if(isbegin_quest(16653) == 1){
		cutin "ep18_miriam_01.png",2;
		mes "[Miriam]";
		mes "How is it going?";
		mes "I trust you. I'll leave it up to you.";
		close3;
	}
	if(isbegin_quest(16654) == 1){
		cutin "ep18_miriam_01.png",2;
		mes "[Miriam]";
		mes "The adventurer might be a senior Rgan, but still a newborn. It'll look suspicious if you try to get too involved.";
		next;
		cutin "ep19_leizi03.png",2;
		mes "[Lazy]";
		mes "But, Argh! It's frustrating not to know what to discuss.";
		mes "Adventurer, I think we need to speed things up.";
		next;
		mes "[Lazy]";
		mes "If we walk together, we might be found out the we are working under the water.";
		mes "The fact that Bagot made a move, means that he may have noticed something.";
		next;
		cutin "ep19_leizi02.png",2;
		mes "[Lazy]";
		mes "Let's persuade Juncea first.";
		mes "If you she can't be persuaded, even in a coercive way... you already know what I'm talking about, right?";
		next;
		cutin "ep18_miriam_01.png",2;
		mes "[Miriam]";
		mes "We need to get her somehow.";
		mes "If you use a Rgan transformation scroll, you'll be less noticeable, whether you're persuading or using force.";
		next;
		cutin "ep19_leizi02.png",2;
		mes "[Lazy]";
		mes "We need to hurry.";
		mes "Let's go back to <NAVI>[Juncea]<INFO>jor_nest,19,190,0,101,0</INFO></NAVI> immediately.";
		next;
		mes "[Lazy]";
		mes "Now that we've all rested, we'll have to go and finish what we were doing.";
		mes "My back, it hurts when I transform into a Rgan.";
		completequest 16654;
		setquest 16655;
		close2;
		cutin "",255;
		cloaknpc("Lazy#ep19re2",true,getcharid(0));
		cloaknpc("Miriam#ep19re1",true,getcharid(0));
		end;
	}
	end;
}

jor_nest,19,190,3	script	Unnoticed Box#ep19re1	4_WOODBOX,{
	if(!191_scroll_check()){
		mes "[" + strcharinfo(0) + "]";
		mes "(Whoops, the transformation scroll! Let's not forget to use it.)";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	if(isbegin_quest(16655) == 1){
		mes "[" + strcharinfo(0) + "]";
		mes "(There's no one here? Where did Juncea go?)";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "(This box, I haven't noticed it before. There is something coming out of it. It looks like a liquid.... it looks a little suspicious. Doesn't it have a <NAVI>[container]<INFO>jor_nest,20,194,0,101,0</INFO></NAVI> where I can put it in to? Let's look around.)";
		completequest 16655;
		setquest 16656;
		questinfo_refresh;
		close2;
		cloaknpc("Empty Test Tube#ep19re1",false,getcharid(0));
		end;
	}
	if(isbegin_quest(16656) == 1){
		mes "[" + strcharinfo(0) + "]";
		mes "(The suspicious liquid is still buoyant. Should we move it around a little?)";
		next;
		if(select("No need to move it.:Let's move it.") == 1){
			mes "[" + strcharinfo(0) + "]";
			mes "(I don't need it anymore.)";
			close;
		}
		mes "[" + strcharinfo(0) + "]";
		mes "(I want to carry the suspicious liquid, but I don't have the container to carry it. Let's look around.)";
		close;
	}
	if(isbegin_quest(16657) == 1){
		mes "[" + strcharinfo(0) + "]";
		mes "(I transferred the suspicious liquid into the test tube.)";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "(Now I have to sneak out..., ah, I can feel some movement outside. Let's walk out pretending that I don't know anything.)";
		delitem 1000604,1;
		getitem 1000605,1;
		completequest 16657;
		setquest 16658;
		questinfo_refresh;
		close2;
		cloaknpc("Rgan#ep19re1",false,getcharid(0));
		end;
	}
	mes "[" + strcharinfo(0) + "]";
	mes "(The liquid is still buoyant.)";
	close;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16655) == 1";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16657) == 1";
end;
}

jor_nest,20,194,3	script	Empty Test Tube#ep19re1	4_POINT_BLUE,{
	if(isbegin_quest(16656) == 1){
		mes "[" + strcharinfo(0) + "]";
		mes "(There is a clean empty test tube. Should I take one?)";
		next;
		if(select("Take it.:Don't take it.") == 2){
			mes "[" + strcharinfo(0) + "]";
			mes "(I won't need it now.)";
			close;
		}
		mes "[" + strcharinfo(0) + "]";
		mes "(I took a clean test tube.)";
		getitem 1000604,1;
		completequest 16656;
		setquest 16657;
		questinfo_refresh;
		close;
	}
	mes "[" + strcharinfo(0) + "]";
	mes "There are several empty test tubes.";
	close;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16656) == 1";
end;
}

jor_nest,24,185,1	script	Rgan#ep19re1	4_EP19_RGAN_SR2,{
	if(!191_scroll_check()){ //TODO
		mes "[" + strcharinfo(0) + "]";
		mes "(Whoops, the transformation scroll! Let's not forget to use it.)"; 
		close2;
		warp "jor_back3",65,321;
		end;
	}
	if(isbegin_quest(16658) == 1){
		mes "[" + strcharinfo(0) + "]";
		mes "...Uh, did you call me?";
		next;
		mes "[Rgan]";
		mes "Yeah. I've never seen you before. You're Senekiogand, right? He said you'd be here anyway.";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "Who? Who said that?";
		next;
		mes "[Rgan]";
		mes "The human scientist. The human named Juncea. She is in Bagot's laboratory right now.";
		mes "She is waiting for you, so maybe you should go?";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "(How did Juncea know that I was coming here? Something's strange.)";
		next;
		mes "[Rgan]";
		mes "They dare to make a Bishop do chores, But I was told to listen to the words of the humans as much as possible, so don't be late.";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "(...there's nothing good to be seen by others now, let's just do what they tell us to do.)";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "I see. <NAVI>[Human Bagot's Laboratory]<INFO>jor_nest,66,260,0,101,0</INFO></NAVI>, right? I'll go there.";
		completequest 16658;
		setquest 16659;
		close2;
		cloaknpc("Rgan#ep19re1",true,getcharid(0));
		end;
	}
	end;

OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16658) == 1";
end;
}


jor_dun03,57,63,3	script	Juncea#ep19re2	4_EP19_JUNCEA,{
	if(!191_scroll_check()){
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "What, eh. Who are you? Where did you come from!";
		close2;
		warp "jor_back3",65,321;
		cutin "",255;
		end;
	}
	if(isbegin_quest(16659) == 1){
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "Oh, I want to talk to you alone. Senekiogand. How does that sound?";
		next;
		cutin "",255;
		mes "- ^0000FFLet's create a one-person party and listen Juncea's story.^000000 -";
		completequest 16659;
		setquest 16660;
		close;
	}
	.@party_id = getcharid(1);
	.@md_name$ = "Bagot's Laboratory";
	if(select("Prepare to enter Bagot's Laboratory.:Enter Bagot's Laboratory") == 1){
		if(!.@party_id || !instance_check_party(.@party_id,1)){
			mes "- ^0000FFOnly the leader of the 1 person party can enter.^000000 -";
			close;
		}
		if(instance_create(.@md_name$) == -3) instance_warning(2,.@md_name$);
		mes "- ^0000FFEntry preparations has begun. After it's ready, select the Enter Bagot's Laboratory button to enter.^000000 -";
		close;
	} else {
		if(!.@party_id || !instance_check_party(.@party_id,1)){
			mes "- ^0000FFOnly the leader of the 1 person party can enter.^000000 -";
			close;
		}
		if(instance_enter(.@md_name$) == IE_OK)
			mapannounce "jor_dun03", strcharinfo(0) + " of the party, "+ getpartyname(.@party_id) +", is entering the " + .@md_name$ + ".", bc_map, "0x00FF99";
		else
			mes "- ^0000FFThe memorial dungeon has not been created yet. Please check again.^000000 -";
		end;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16659) == 1";
end;
}

jor_nest,66,260,3	script	Aroron#ep19re2	4_EP19_IWIN,{
	if(isbegin_quest(17649) == 0) end;
	if(checkquest(16662,HUNTING) == 2){
		cutin "ep19_iwin01.png",2;
		mes "[Aroron]";
		mes "Did you safely win the battle simulation against Juncea?";
		next;
		mes "[Aroron]";
		mes "Oh, you don't have to answer me.";
		mes "I was watching everything from here.";
		next;
		mes "[Aroron]";
		mes "Good job.";
		mes "Winning in an impressive manner.";
		next;
		mes "[Aroron]";
		mes "We have a lot of data, our researchers will love this.";
		mes "Well, I'll get you the promised rewards.";
		close2;
		erasequest 16662;
		setquest 16663;
		getitem 1000608,4;
		getitem 1000811,1;
		end;
	}
	switch(checkquest(16663,PLAYTIME)){
		case -1:
			break;
		
		case 0:
		case 1:
			end;
			
		case 2:
			erasequest 16663;
			break;
	}
	cutin "ep19_iwin01.png",2;
	mes "[Aroron]";
	mes "The battle with Juncea, isn't it thrilling?";
	mes "You want to fight her again, don't you?";
	next;
	cutin "",255;
	if(select("Enter.:I don't want to." ) == 2){
		cutin "ep19_iwin01.png",2;
		mes "[Aroron]";
		mes "Is that right?";
		mes "You're boring.";
		close3;
	}
	cutin "ep19_iwin01.png",2;
	mes "[Aroron]";
	mes "I prepared it for you adventurer.";
	mes "A thrilling battle simulation using Juncea's simulation data.";
	next;
	mes "[Aroron]";
	mes "It's safe, it was prepared in the Ice Castle.";
	mes "We can defeat Juncea once a day.";
	next;
	mes "[Aroron]";
	mes "But you can try again and again until you defeat it.";
	mes "Do you want to try?";
	next;
	.@party_id = getcharid(1);
	if(!.@party_id){
		mes "[Aroron]";
		mes "Oh, make sure you create a party first.";
		mes "It doesn't matter if you're alone or in groups, but make sure you have a party.";
		close3;
	}
	cutin "",255;
	.@md_name$ = "Bagot's Laboratory";
	switch(select((is_party_leader() ? "Prepare Bagot's Laboratory":""),"Enter Bagot's Laboratory","Don't challenge it.")){
		case 1:
			if(instance_create(.@md_name$) == -3) instance_warning(2,.@md_name$);
			mes "- ^0000FFPreparation for entry has begun. When it's ready, press the Enter Bagot's Laboratory button to enter.^000000 -";
			close;
		
		case 2:
			if(instance_enter(.@md_name$) == IE_OK){
				//No enter announcement for some reason
				//mapannounce "jor_nest", strcharinfo(0) + " of the party, "+ getpartyname(.@party_id) +", is entering the " + .@md_name$ + ".", bc_map, "0x00FF99";
				if(!isbegin_quest(16662)) setquest 16662;
			} else
				mes "- ^0000FFThe memorial dungeon has not been created yet. Please check again.^000000 -";
			end;
			
		case 3:
			cutin "ep19_iwin01.png",2;
			mes "[Aroron]";
			mes "Well, you're boring adventurer.";
			close3;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"checkquest(16662,HUNTING) == 2";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(17649) == 2 && isbegin_quest(16662) == 0 && checkquest(16663,PLAYTIME) == -1";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(17649) == 2 && isbegin_quest(16662) == 0 && checkquest(16663,PLAYTIME) == 2";
end;
}

icas_in,247,116,1	script	Horuru#i19ms00	4_EP19_IWIN,5,5,{
	if(isbegin_quest(17640) == 0){
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "If possible, I'd like you to visit only when invited.";
		close3;
	}
	if(isbegin_quest(17640) == 1){
		cutin "ep19_iwin07.png",2;
		mes "[Horuru]";
		mes "I don't think that you've brought this Rgan here for no reason... just what happened?";
		npctalk "What happened?","Lehar#i19ms00",bc_self;
		next;
		cutin "ep19_leizi03.png",1;
		mes "[Lazy]";
		mes "That's right, isn't that the ^0000CDhuman Juncea^000000? Why is she in that shape?";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "I had no choice but to ger her out of the Rgan's nest safely.";
		next;
		cutin "ep19_leizi03.png",1;
		mes "[Lazy]";
		mes "Lie her down over there for now.";
		npctalk "For a moment there, did you think this is your house, Lazy?","Horuru#i19ms00",bc_self;
		next;
		cutin "ep19_lehar05.png",1;
		mes "[Lehar]";
		mes "You said that you brought her out, are we just going to keep her in a shape like this?";
		next;
		cutin "ep19_leizi03.png",1;
		mes "[Lazy]";
		mes "Oh, let's talk about that later, change her into human form for now. That way, we can get her some treatment or something.";
		next;
		specialeffect EF_FOOD02,AREA,"Juncea#i19ms00";
		specialeffect EF_CHAINCOMBO,AREA,"Juncea#i19ms00";
		cutin "",255;
		sleep2 1000;
		cloaknpc("Juncea#i19ms00",true,getcharid(0));
		cloaknpc("Juncea#i19ms01",false,getcharid(0));
		sleep2 1000;
		cutin "ep18_miriam_01.png",1;
		mes "[Miriam]";
		mes "This body... has wings...!";
		next;
		cutin "ep19_leizi03.png",1;
		mes "[Lazy]";
		mes "Hey newphew, don't you have god's power? Please give her some treatment.";
		npctalk "Ye, Yes!!","Lehar#i19ms00",bc_self;
		sleep2 1000;
		specialeffect EF_HEAL4,AREA,"Juncea#i19ms01";
		sleep2 500;
		specialeffect EF_CURE,AREA,"Juncea#i19ms01";
		sleep2 500;
		specialeffect EF_RECOVERY ,AREA,"Juncea#i19ms01";
		next;
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "Now explain the situation.";
		next;
		select("I think our plan was found out.");
		cutin "ep19_leizi03.png",1;
		mes "[Lazy]";
		mes "Well, I don't know how, but I guess he found out that someone was trying to deceive them?";
		next;
		cutin "ep18_miriam_01.png",1;
		mes "[Miriam]";
		mes "But making your colleague like this to get rid of the other person, isn't it too much?";
		next;
		cutin "ep19_lehar05.png",1;
		mes "[Lehar]";
		mes "But this body.... How did this happen?";
		next;
		select("I think it's related to Bagot's experiment.");
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "It feels like the false god chimera that was made in Rachel.";
		next;
		cutin "ep18_miriam_01.png",1;
		mes "[Miriam]";
		mes "Then, Bagot's research... It can't be...!";
		next;
		cutin "ep19_leizi03.png",1;
		mes "[Lazy]";
		mes "Whatever it may be, it won't be good for us. Let's wake her up first and ask for the details.";
		next;
		select("Don't just bump her.");
		cutin "ep19_leizi03.png",1;
		mes "[Lazy]";
		mes "Hey. Human Juncea. Wake up. I have a lot to ask.";
		npctalk "Juncea is a human?","Lehar#i19ms00",bc_self;
		next;
		cutin "",255;
		mes "[Juncea]";
		mes "......";
		next;
		cutin "ep19_leizi03.png",1;
		mes "[Lazy]";
		mes "What should we do? She can't even get it together?";
		mes "How long has she been unconcious? Why doesn't she return to her original form?";
		next;
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "How did this happen? Are there any clues?";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "Hm...... I picked this up in the laboratory, but I don't know if it'll help.";
		next;
		cutin "ep19_leizi01.png",1;
		mes "[Lazy]";
		mes "Can anyone investigate this?";
		next;
		cutin "ep19_lehar01.png",1;
		mes "[Lehar]";
		mes "What, is this? Is this something edible?";
		next;
		cutin "ep19_iwin07.png",2;
		mes "[Horuru]";
		mes "No, I know what it is! It maybe a sauce!";
		next;
		cutin "ep19_leizi03.png",1;
		mes "[Lazy]";
		mes "Unfortunately, none of us know what this is. It's from that place, so it may be give us a clue to the detoxification.";
		next;
		cutin "ep19_lehar02.png",1;
		mes "[Lehar]";
		mes "Oh! If that's the case, I'll bring my aunt. She's good at things like this.";
		npctalk "Oh, a scientist. yeah.","Lazy#i19ms00",bc_self;
		next;
		cloaknpc("Lehar#i19ms00",true,getcharid(0));
		cutin "ep19_leizi01.png",1;
		mes "[Lazy]";
		mes "Anyway, I didn't expect us to be caught so soon. Our faces are probably known, so we can't show our faces there anymore.";
		next;
		cutin "ep19_iwin07.png",2;
		mes "[Horuru]";
		mes "We can't use the transformation scroll method to infiltrate anymore.";
		next;
		cutin "ep19_leizi01.png",1;
		mes "[Lazy]";
		mes "That's not it, Mr. Iwin. Didn't you said that there are only a few senior and superior Rgans so they know the faces of each one? You can still continue using that method as long you're an intermediate Rgan.";
		next;
		cutin "ep19_leizi03.png",1;
		mes "[Lazy]";
		mes "But the information that an intermediate Rgan can get is a problem... The range of access is smaller.";
		next;
		cloaknpc("Vellgunde#i19ms00",false,getcharid(0));
		cloaknpc("Lehar#i19ms00",false,getcharid(0));
		cutin "ep19_vellgunde01.png",2;
		mes "[Vellgunde]";
		mes "Did you need me? Take a look at something...";
		mes "Ho, This body...? Is it a result of an experimet from an Illusion scientist? Did he use his colleague as an experimental subject?";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "Yes, we need this person to return to normal, but I don't know how. This is...";
		next;
		cutin "ep19_vellgunde02.png",2;
		mes "[Vellgunde]";
		mes "I'll take a look.";
		mes "Ho... Hm...... Uhm... ";
		next;
		cutin "ep19_vellgunde01.png",2;
		mes "[Vellgunde]";
		mes "I know the gist of it. Fortunately, I think I've got the things I need. We can get the rest separately.";
		npctalk "As expected of a scientist!!!","Lazy#i19ms00",bc_self;
		next;
		cutin "ep19_vellgunde02.png",2;
		mes "[Vellgunde]";
		mes "Bring me a ^0000CDRgan Low-Grade Mana Core^000000. As many as possible. ^0000CD20 or more^000000.";
		next;
		select("Is that everything?");
		cutin "ep19_vellgunde02.png",2;
		mes "[Vellgunde]";
		mes "Yes. There's nothing good if we delay too much, so why don't you go right away?";
		delitem 1000605,1;
		completequest 17640;
		setquest 17641;
		questinfo_refresh;
		close3;
	}
	if(isbegin_quest(17641) == 1){
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "We need ^0000CD20 Rgan Low-Grade Mana Core^000000. Adventurer will suffer a bit. I wanted to go, but I can't leave the house at the moment.";
		close3;
	}
	if(isbegin_quest(17642) == 1){
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "What is she saying now?";
		close3;
	}
	if(isbegin_quest(17644) == 1){
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "We've detected abnormalities in the Rgan's nest, so we'd better get going.";
		close3;
	}
	end;
	
OnTouch:
	if(isbegin_quest(17640) == 1){
		cloaknpc("Juncea#i19ms00",false,getcharid(0));
		cloaknpc("Lazy#i19ms00",false,getcharid(0));
		cloaknpc("Lehar#i19ms00",false,getcharid(0));
		cloaknpc("Miriam#i19ms00",false,getcharid(0));	
	}
	if(isbegin_quest(17641) == 1){
		cloaknpc("Juncea#i19ms01",false,getcharid(0));
		cloaknpc("Lazy#i19ms00",false,getcharid(0));
		cloaknpc("Lehar#i19ms00",false,getcharid(0));
		cloaknpc("Miriam#i19ms00",false,getcharid(0));
		cloaknpc("Juncea#i19ms01",false,getcharid(0));
		cloaknpc("Vellgunde#i19ms00",false,getcharid(0));
	}
	if(isbegin_quest(17642) == 1){
		cloaknpc("Juncea#i19ms02",false,getcharid(0));
		cloaknpc("Lazy#i19ms01",false,getcharid(0));
		cloaknpc("Lehar#i19ms00",false,getcharid(0));
		cloaknpc("Miriam#i19ms00",false,getcharid(0));
		cloaknpc("Vellgunde#i19ms00",false,getcharid(0));
	}
	if(isbegin_quest(17643) == 1){
		cloaknpc("Juncea#i19ms02",false,getcharid(0));
		cloaknpc("Lazy#i19ms00",false,getcharid(0));
		cloaknpc("Lehar#i19ms00",false,getcharid(0));
		cloaknpc("Miriam#i19ms00",false,getcharid(0));
		cloaknpc("Vellgunde#i19ms00",false,getcharid(0));
		cloaknpc("Aurelie#i19ms00",false,getcharid(0));
		cloaknpc("Iwin#i19ms00",false,getcharid(0));
	}
	if(isbegin_quest(17643) == 2)
		cloaknpc("Juncea#i19ms02",false,getcharid(0));
end;

OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17640) == 1";
end;
}

icas_in,244,117,1	script	Vellgunde#i19ms00	4_EP19_VELLGUNDE,{
	if(isbegin_quest(17641) == 1){
		if(countitem(1000707) < 20){
			cutin "ep19_vellgunde02.png",2;
			mes "[Vellgunde]";
			mes "This is not enough. We need plenty of it.";
			mes "Bring me 20 Rgan Low-Grade Mana Core.";
			close3;
		}
		cutin "ep19_vellgunde02.png",2;
		cloaknpc("Juncea#i19ms01",false,getcharid(0));
		mes "[Vellgunde]";
		mes "Yes, well done.";
		mes "Now, we can just combine it according to the recipe.";
		next;
		specialeffect EF_PRIMECHARGE ,AREA,"Juncea#i19ms01";
		cutin "ep19_vellgunde01.png",2;
		mes "[Vellgunde]";
		mes "Now, shall we inject it? You'd better step back just in case.";
		next;
		cutin "",255;
		mes "[Juncea]";
		mes "Ugh......";
		next;
		cutin "ep19_vellgunde01.png",2;
		mes "[Vellgunde]";
		mes "Sh- not good. Slowly... slowly... yes, just relax your body and don't rush.";
		next;
		specialeffect EF_HITLINE8,AREA,"Juncea#i19ms01";
		cutin "",255;
		npctalk "Ugh...............","Juncea#i19ms01",bc_self;
		npctalk "Oh, she's returning to her original form!","Horuru#i19ms00",bc_self;
		npctalk "A scientist indeed. Is not for show.","Lazy#i19ms00",bc_self;
		npctalk "What does being a scientist have to do with it?","Lehar#i19ms00",bc_self;
		sleep2 1500;
		specialeffect EF_HITLINE8,AREA,"Juncea#i19ms01";
		specialeffect EF_CHAINCOMBO,AREA,"Juncea#i19ms01";
		sleep2 1000;
		cloaknpc("Juncea#i19ms01",true,getcharid(0));
		cloaknpc("Juncea#i19ms02",false,getcharid(0));
		cutin "ep19_juncea04.png",0;
		mes "[Juncea]";
		mes "Ugh...... I......";
		next;
		cutin "ep19_vellgunde01.png",2;
		mes "[Vellgunde]";
		mes "So,are you finally awake? You. Who are you? Do you know who you are?";
		next;
		cutin "",255;
		specialeffect EF_SCREEN_QUAKE,AREA,"Juncea#i19ms02";
		mes "[Juncea]";
		mes "<FONT SIZE = 18><B><I>Bagot!!!!</I></B></FONT>";
		npctalk "Bagot? Didn't you say she's Juncea?","Miriam#i19ms00",bc_self;
		unittalk getcharid(3),strcharinfo(0) + " : " + "No. She's just resenting her.",bc_self;
		next;
		specialeffect EF_SCREEN_QUAKE,AREA,"Juncea#i19ms02";
		mes "[Juncea]";
		mes "<FONT SIZE = 18><B><I>You used me as a subject!!!</I></B></FONT>";
		npctalk "To have this energy as soon as she wake up. Well, your nephew must have really strong powers.","Lazy#i19ms00",bc_self;
		npctalk "Haha, what do you mean.","Lehar#i19ms00",bc_self;
		next;
		cutin "",255;
		mes "[Juncea]";
		mes "<FONT SIZE = 18><B><I>Bagot!!!</I></B></FONT>";
		next;
		mes "[Juncea]";
		mes "<FONT SIZE = 18><B><I>Bagot!!! <FONT SIZE = 24>Bagot!!!</I></B></FONT>";
		next;
		cutin "ep19_vellgunde02.png",2;
		mes "[Vellgunde]";
		mes "This is not the Rgan's Nest. So don't go looking for him.";
		npctalk "You better calm down. The man you're looking for is not here.","Lehar#i19ms00",bc_self;
		next;
		cutin "ep19_juncea02.png",0;
		mes "[Juncea]";
		mes "......? Where, am I? The nest... No?";
		next;
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "This is the Ice Castle. As you can see, I'm an Iwin and they're humans. We saved you from the snake's nest.";
		next;
		cutin "ep19_leizi01.png",1;
		mes "[Lazy]";
		mes "Hi! We've met before! Can you recognize me? It's Svegand!";
		next;
		select("I'm... Senekiogand.");
		cutin "ep19_juncea02.png",0;
		mes "[Juncea]";
		mes "What...? You guys were humans? How, did you do it...? You were definitely a Rgan.";
		next;
		cutin "ep19_leizi02.png",1;
		mes "[Lazy]";
		mes "Well, there was a super genius who's on our side unintentionally.";
		npctalk "And you people just used that kind of person as an experimental subject.","Lazy#i19ms00",bc_self;
		next;
		cutin "ep19_juncea02.png",0;
		mes "[Juncea]";
		mes "Transformation? Human, into a Rgan?";
		mes "You've been deceiving me all this time? Did you use me?";
		next;
		cutin "ep19_leizi02.png",1;
		mes "[Lazy]";
		mes "Why don't you tell me a bad reason for separating and bringing you back here? It all turned out well, didn't it?";
		npctalk "You want an apology?","Lazy#i19ms00",bc_self;
		npctalk "An apology! Those people are criminals!","Horuru#i19ms00",bc_self;
		next;
		cutin "ep19_juncea03.png",0;
		mes "[Juncea]";
		mes "What are you going to do with me? Are you going to dispose me? You'll gain nothing even if you get rid of me.";
		next;
		cutin "ep19_leizi01.png",1;
		mes "[Lazy]";
		mes "I've never seen anyone arrested before who doesn't speak that line.";
		npctalk "I made a bet with myself that it won't happen!","Lazy#i19ms00",bc_self;
		next;
		select("Please be quiet.");
		cutin "ep19_juncea04.png",0;
		mes "[Juncea]";
		mes "Did Bagot sold me out?? Since when did you work together?";
		mes "Ugh--";
		next;
		cutin "ep19_vellgunde01.png",2;
		mes "[Vellgunde]";
		mes "So where else can you go?";
		mes "You need to be treated well. It's not a minor injury.";
		next;
		cutin "ep19_leizi02.png",1;
		mes "[Lazy]";
		mes "I would never cooperate with Bagot. Isn't it the for you? If so, why have I brought you here?";
		mes "Just eat something and recuperate for now. You haven't got any decent meal there, right? And we'll have you cooperate with us from now on.";
		next;
		cutin "ep19_juncea04.png",0;
		mes "[Juncea]";
		mes "You want me to eat......! How can I! How dare you! ?!!";
		next;
		cutin "ep19_leizi03.png",1;
		mes "[Lazy]";
		mes "Hey, we went out there to save you. You'd be dead already if we didn't go. Would you rather die than live?";
		npctalk "No way again.","Lazy#i19ms00",bc_self;
		npctalk "......","Juncea#i19ms02",bc_self;
		next;
		cutin "ep19_leizi03.png",1;
		mes "[Lazy]";
		mes "Then cooperate with us if that's not the case. If you can tell me that without hesistation, then I'll make a deal with you.";
		mes "Don't forget that the Illusions are wanted in the three kingdom of Midgars. There's nowhere to run.";
		npctalk ".........","Juncea#i19ms02",bc_self;
		next;
		cutin "ep19_leizi03.png",1;
		mes "[Lazy]";
		mes "He escaped us and fled all the way here, but we followed him all the way here. Where next? Do you have a place to go? Nowhere, right?";
		npctalk "............","Juncea#i19ms02",bc_self;
		next;
		cutin "ep19_leizi03.png",1;
		mes "[Lazy]";
		mes "Look. We're not doing this because we're desperate. It's because we don't want to waste resources on unnecessary battles..";
		mes "If you don't speak out now, I'll go back to the laboratory and destroy everything in it.";
		npctalk "I heard researchers don't like that? Don't like their stuffs getting touched.","Lazy#i19ms00",bc_self;
		next;
		cutin "ep19_juncea02.png",0;
		mes "[Juncea]";
		mes "Transformation......? Are you going to use that again?";
		next;
		cutin "ep19_leizi03.png",1;
		mes "[Lazy]";
		mes "Oh, yeah. Why? Are you interested? Oh, you've never seen me transform before. Curious? I can show it if you want.";
		next;
		cutin "",255;
		specialeffect EF_FOOD06,AREA,"Lazy#i19ms00";
		cloaknpc("Lazy#i19ms00",true,getcharid(0));
		cloaknpc("Lazy#i19ms01",false,getcharid(0));
		mes "[Lazy]";
		mes "Tada!!! How is it! It's exactly the same, right?";
		next;
		cutin "ep19_juncea02.png",0;
		mes "[Juncea]";
		mes "A human, into a Rgan... it's definitely a Rgan. The mana wavelength, organs.. How......?";
		next;
		cutin "ep19_vellgunde02.png",2;
		mes "[Vellgunde]";
		mes "I can explain it to you. Don't you want to satisfy your curiosity as a scientist?";
		mes "Instead, there's something you need to tell us.";
		next;
		cutin "ep19_juncea03.png",0;
		mes "[Juncea]";
		mes "...I'll think about it.";
		delitem 1000707,20;
		completequest 17641;
		setquest 17642;
		questinfo_refresh;
		getitem 1000608,25;
		close3;
	}
	if(isbegin_quest(17644) == 1){
		cutin "ep19_vellgunde01.png",2;
		mes "[Vellgunde]";
		mes "I'll watch Juncea a bit more. You'll never know when we'll need her, right?";
		close3;
	}
	cutin "ep19_vellgunde01.png",2;
	mes "[Vellgunde]";
	mes "Persuading her doesn't seem difficult, or is it? What do you think? Adventurer?";
	close3;

OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17641) == 1 && countitem(1000707) >= 20";
end;
}

icas_in,243,120,5	script	Juncea#i19ms02	4_EP19_JUNCEA,{
	if(isbegin_quest(17642) == 1){
		cutin "ep19_juncea03.png",0;
		mes "[Juncea]";
		mes "Okay. I accept your offer. The treatment you gave earlier, Promise it.";
		npctalk "Return to your original form! No Rgans in my house!","Horuru#i19ms00",bc_self;
		npctalk "Well, you're stingy.","Lazy#i19ms00",bc_self;
		next;
		specialeffect EF_CHAINCOMBO,AREA,"Lazy#i19ms00";
		cloaknpc("Lazy#i19ms01",true,getcharid(0));
		cloaknpc("Lazy#i19ms00",false,getcharid(0));
		cutin "ep19_leizi01.png",1;
		mes "[Lazy]";
		mes "Of course. I'm fully supported by the government of Schwarzwald, so you can count on me. Not only me, isn't everyone here representing one of the three kingdoms?";
		next;
		cutin "ep19_juncea03.png",0;
		mes "[Juncea]";
		mes "You told me there's no one I can trust in the world.";
		npctalk "Wherever can you find an icon of trust like myself?","Lazy#i19ms00",bc_self;
		next;
		cutin "ep19_leizi02.png",1;
		mes "[Lazy]";
		mes "You're not the only Illusion who traded sentences for cooperation. The Illusion that we caught earlier is also in a safe place.";
		npctalk "Forget that name.","Juncea#i19ms02",bc_self;
		npctalk "Ouch! Let's do that. it's not difficult.","Lazy#i19ms00",bc_self;
		next;
		cutin "ep19_juncea03.png",0;
		mes "[Juncea]";
		mes "That's a detention. It's a classy way to put a criminal in jail. It's a prison sentence basically.";
		mes "What kind of generosity and acceptance does it give by not killing them. Don't go off pretending to show sentiment.";
		next;
		cutin "ep19_lehar02.png",1;
		mes "[Lehar]";
		mes "Oh, how about this? Does Bagot have any sentinment? We'll take that revenge for you. You, Juncea will be a traitor to them.";
		npctalk "No, is this really your nephew?","Lazy#i19ms00",bc_self;
		next;
		cutin "ep19_juncea02.png",0;
		mes "[Juncea]";
		mes "Revenge... traitor... I like it. Traitor... I've been betrayed, why can't I betray them?";
		npctalk "But it won't be good if you betray us.","Lazy#i19ms00",bc_self;
		next;
		cutin "ep19_juncea04.png",0;
		mes "[Juncea]";
		mes "Okay. Now, what are you curious about?";
		next;
		cutin "ep19_leizi01.png",1;
		mes "[Lazy]";
		mes "How powerful is the Jormungandr cult?";
		next;
		cutin "ep19_juncea04.png",0;
		mes "[Juncea]";
		mes "As you may have seen, they are numerous and can create theirselves indefinitely, as long as they have blood and time.";
		next;
		cutin "ep19_juncea01.png",0;
		mes "[Juncea]";
		mes "However, it needs mana to make it hatch and gain power, but the source of their mana, ^0000CDJormungandr's main body^000000 is running out of mana, so the future is not looking good for them.";
		next;
		cutin "ep19_leizi03.png",1;
		mes "[Lazy]";
		mes "The main body of Jormungandr?";
		next;
		cutin "ep19_juncea01.png",0;
		mes "[Juncea]";
		mes "The main body is where the Rgan made their base. That place is a body, it got a lot of residual mana.";
		mes "But the remaining mana is not infinite, there is only a little left since they've been using it for a long time.";
		next;
		cutin "ep19_vellgunde01.png",2;
		mes "[Vellgunde]";
		mes "Lasgand must have been worried about it.";
		mes "Is that why he reached out of the continent and joined hands with the Illusion?";
		next;
		select("How is their power?");
		cutin "ep19_juncea01.png",0;
		mes "[Juncea]";
		mes "As you saw, the senior and superior Rgans are strong. They already had lots of mana from the beginning, but grew even more by absorbing a lot of it.";
		mes "But, not to the point that you won't be able to deal with it.";
		next;
		cutin "ep19_juncea04.png",0;
		mes "[Juncea]";
		mes "There aren't many superior ones. Besides, their mana is almost dry, so it's possible if we have enough time. Lasgand is the only one we should watch for.";
		next;
		cutin "ep19_juncea01.png",0;
		mes "[Juncea]";
		mes "Lasgand is a long time survivor, he must have a lot of mana. I'm sure the mana he absorbed all his life is not easy to handle.";
		mes "That's why all the superior Rgans was born from the blood he sowed.";
		next;
		cutin "ep19_juncea04.png",0;
		mes "[Juncea]";
		mes "I don't know much about him actually. I wanted to study him before, but I wasn't allowed to touch the bodies.";
		next;
		cutin "ep19_vellgunde02.png",2;
		mes "[Vellgunde]";
		mes "Hm, Lasgand is the key. The final survivor...... the one who put the curse on the ^0000CDroyal families^000000. To think that such a thing would cooperate with the Illusions, and even favors them.";
		next;
		cutin "ep19_vellgunde01.png",2;
		mes "[Vellgunde]";
		mes "What did he ask from the Illusions? I heard that you studied the amplification of the remaining mana?";
		next;
		cutin "ep19_juncea01.png",0;
		mes "[Juncea]";
		mes "What I studied was the body of Jormungandr and it's mana.";
		mes "If it was able to contain such mana for a long time, it can contain it for more longer.";
		next;
		cutin "ep19_leizi03.png",1;
		mes "[Lazy]";
		mes "That's why Lasgand gave full support. But, Bagot took the opportunity away.";
		mes "What did he offer?";
		next;
		cutin "ep19_juncea03.png",0;
		mes "[Juncea]";
		mes "Perhaps... the resurrection of Jormungandr.";
		npctalk "Resurrection...!","Lazy#i19ms00",bc_self;
		npctalk "Resurrection?? Does that make any sense?","Lehar#i19ms00",bc_self;
		npctalk "As expected, I see...","Vellgunde#i19ms00",bc_self;
		npctalk "...!!!","Horuru#i19ms00",bc_self;
		next;
		cutin "ep19_juncea01.png",0;
		mes "[Juncea]";
		mes "Their interest. It's easy to predict it if you think about what they did to me.";
		mes "Wi the remaining body and the Heart of Ymir, they might be able to ^0000cdressurect Jormungandr^000000 or if it fails, he'll ^0000CDrecreate the deity Jormungandr^000000.";
		next;
		select("If you think about what happened in Arunafeltz...");
		cutin "ep19_juncea03.png",0;
		mes "[Juncea]";
		mes "I don't know how complete Bagot's experiment is, but it's making a lot of progress... it's probably on the verge of completion.";
		next;
		cutin "ep19_leizi03.png",1;
		mes "[Lazy]";
		mes "The resurrection of Jormungandr...... just the tought of it is enough to make me faint.";
		next;
		cutin "ep18_miriam_01.png",1;
		mes "[Miriam]";
		mes "Wait, what will Bagot get from Lasgand for doing that?";
		next;
		cutin "ep19_juncea04.png",0;
		mes "[Juncea]";
		mes "He's crazy, the research itself is what he wants to do.";
		mes "He'll do whatever he needs to just to get it absolutely in his hands. And it just happened to match Lasgand's desire.";
		next;
		cutin "ep19_vellgunde03.png",2;
		mes "[Vellgunde]";
		mes "If it resurrected under their hands, The continent will once again be in a big crisis.";
		npctalk "We don't have much time as you think!!!","Lehar#i19ms00",bc_self;
		next;
		cutin "ep19_leizi03.png",1;
		mes "[Lazy]";
		mes "We're going to have to make a plan right now. Now, tell me everything you know about the snake's nest.";
		next;
		cutin "ep19_juncea01.png",0;
		mes "[Juncea]";
		mes "As far as I know there's only one entrance. We need to preempt the area where the low-grade Rgans are located, and then block the middle passage.";
		mes "They'll be stranded with that, so they'll either come out through secret passages or dig the ground.";
		next;
		cutin "ep19_iwin06.png",2;
		mes "[Horuru]";
		mes "We can surround the place and attack those who comes out.";
		next;
		cutin "ep19_juncea01.png",0;
		mes "[Juncea]";
		mes "Use the scrolls that you have. I'm sure they'll be careless if you're the same Rgan as them.";
		next;
		cutin "ep19_leizi01.png",1;
		mes "[Lazy]";
		mes "Oh, is this the taste of betrayal that you want?";
		next;
		cloaknpc("Aurelie#i19ms00",false,getcharid(0));
		cloaknpc("Iwin#i19ms00",false,getcharid(0));
		cutin "ep19_aurelie03.png",2;
		mes "[Aurelie]";
		mes "Everyone is here. Who...?";
		next;
		cutin "ep19_leizi01.png",1;
		mes "[Lazy]";
		mes "It's a criminal captured by the adventurer.";
		next;
		cutin "ep19_vellgunde01.png",2;
		mes "[Vellgunde]";
		mes "What's the matter?";
		completequest 17642;
		setquest 17643;
		close3;
	}
	if(isbegin_quest(17643) == 1){
		cutin "ep19_juncea04.png",0;
		mes "[Juncea]";
		mes "What kind of a meeting place is this? Why are you gathering in such a small place?";
		close3;
	}
	if(isbegin_quest(17644) == 1){
		cutin "ep19_juncea01.png",0;
		mes "[Juncea]";	
		mes "Don't forget the transformation scroll when you into the snake's nest. ...and don't forget to show it to me later.";
		close3;
	}
	cutin "ep19_juncea03.png",0;
	mes "[Juncea]";
	mes "Are you here to interrogate me? Don't touch me. I went too far and I'm not in a shape to do anything right now. Please let me take a little break.";
	close3;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17642) == 1";
end;
}

icas_in,243,120,5	duplicate(dummynpc2)	Juncea#i19ms00	4_EP19_RGAN_R1
icas_in,243,120,3	duplicate(dummynpc2)	Juncea#i19ms01	4_EP19_JUNCEA_D

icas_in,249,113,3	script	Aurelie#i19ms00	4_EP19_AURELIE,{
	if(isbegin_quest(17643) == 1){
		cutin "ep19_aurelie03.png",2;
		mes "[Aurelie]";
		mes "There have been reports from the reconnaissance unit about suspicious movements from the Rgans.";
		next;
		cutin "ep19_iwin02.png",2;
		mes "[Iwin]";
		mes "The Rgans are rushing to somewhere";
		next;
		select("Did they get any orders?");
		cutin "ep19_aurelie03.png",2;
		mes "[Aurelie]";
		mes "It seems so, judging from their movement. Even the senior Rgans who were scouting also went back.";
		next;
		cutin "ep19_leizi03.png",1;
		mes "[Lazy]";
		mes "Does anyone know what's happening? I think now is the right time for us to move. We should go now.";
		next;
		cutin "ep19_iwin08.png",2;
		mes "[Horuru]";
		mes "We need to take advantage of the confusion to bring chaos. We need to hurry and find out.";
		next;
		cutin "ep19_juncea04.png",0;
		mes "[Juncea]";
		mes "The scroll, don't forget it. If they all gather, no one will know your faces, so it'll be useful.";
		completequest 17643;
		setquest 17644;
		close2;
		cutin "",255;
		cloaknpc("Aurelie#i19ms00",true,getcharid(0));
		cloaknpc("Lazy#i19ms00",true,getcharid(0));
		cloaknpc("Lehar#i19ms00",true,getcharid(0));
		cloaknpc("Miriam#i19ms00",true,getcharid(0));
		cloaknpc("Iwin#i19ms00",true,getcharid(0));
		end;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17643) == 1";
end;
}

icas_in,245,115,1	script	Miriam#i19ms00	4_EP18_MIRIAM,{
	if(isbegin_quest(17640) == 1){
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "What's going on? That one, isn't she an Illusion?";
		close3;
	}
	if(isbegin_quest(17641) == 1){
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "A Low-Grade Mana Core, I've seen it in the warmth of the snake god.";
		close3;
	}
	cutin "ep18_miriam_01.png",0;
	mes "[Miriam]";
	mes "Will we able to catch Bagot and get the Heart of Ymir back.";
	close3;
}

icas_in,248,118,3	script	Lehar#i19ms00	4_EP19_LEHAR,{
	if(isbegin_quest(17640) == 1){
		cutin "ep19_lehar01.png",1;
		mes "[Lehar]";
		mes "Me? I've been invited here. We've got to know each other in the Rgan's nest. But what happened to that person?";
		close3;
	}
	if(isbegin_quest(17641) == 1){
		cutin "ep19_lehar01.png",1;
		mes "[Lehar]";
		mes "We we need is Low-Grade Mana Core. The writing was so messy I couldn't read it immediately.";
		close3;
	}
	cutin "ep19_lehar01.png",1;
	mes "[Lehar]";
	mes "It as expected, from my aunt. By the way, what is that person saying?";
	close3;
}

icas_in,246,119,3	script	Lazy#i19ms00	4_EP19_LAZY,{
	if(isbegin_quest(17640) == 1){
		cutin "ep19_leizi03.png",1;
		mes "[Lazy]";
		mes "Just what happened to the human Juncea while we weren't there?";
		close3;
	}
	if(isbegin_quest(17641) == 1){
		cutin "ep19_leizi03.png",1;
		mes "[Lazy]";
		mes "It's easy bringing 20 Low-Grade Mana Core for you adventurer, right? I mean, is there 30000 items in your bag right now?";
		close3;
	}
	if(isbegin_quest(17643) == 1){
		cutin "ep19_leizi03.png",1;
		mes "[Lazy]";
		mes "I think all the criminals from the Illusion are quite the same. Maybe that's why they're easy to handle?";
		close3;
	}
	end;
}

icas_in,246,119,3	duplicate(dummynpc2)	Lazy#i19ms01	4_EP19_RGAN_SR3

icas_in,247,113,1	script	Iwin#i19ms00	4_EP19_IWIN,{
	cutin "ep19_iwin02.png",2;
	mes "[Iwin]";
	mes "It seems that something strange has occured!";
	close3;
}

jor_dun01,8,238,0	script	#to_jor_dun02	WARPNPC,1,1,{
	end;
	
OnTouch:
	if(isbegin_quest(17644) > 0 && isbegin_quest(17648) < 2){
		cloaknpc("Iwin Recon Unit#epm00",false,getcharid(0));
		mes "[Iwin Recon Unit]";
		mes "Adventurer! Are you entering?";
		mes "If you start a search operation now, you'll have to put all your efforts into it. The atmosphere over there is bad.";
		next;
		mes "^FF0000※ If you start a search operation, you will not be able to enter the existing 2nd floor map of the snake's nest until the operation is completed.^000000";
		next;
		if(select("Enter now.:I need to prepare.") == 2){
			mes "[Iwin Recon Unit]";
			mes "Okay. Then prepare and enter!";
			close;
		}
		mes "[Iwin Recon Unit]";
		mes "Goodluck!";
		close2;
		cloaknpc("Iwin Recon Unit#epm00",true,getcharid(0));
		warp "jor_que",282,87;
		end;
	}
	warp "jor_dun02",282,87;
end;

OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17644) > 0 && isbegin_quest(17648) < 2";
end;
}

jor_dun01,12,240,3	duplicate(dummynpc2)	Iwin Recon Unit#epm00	21529

jor_que,281,87,0	script	#i19ms11	HIDDEN_WARP_NPC,5,5,{
	end;
	
OnTouch:
	if(isbegin_quest(17644) == 1){
		cloaknpc("Horuru#i19ms11",false,getcharid(0));
		cloaknpc("Miriam#i19ms11",false,getcharid(0));
		cloaknpc("Lehar#i19ms11",false,getcharid(0));
		cloaknpc("Lazy#i19ms11",false,getcharid(0));
		cloaknpc("Iwin Recon#i19ms12",false,getcharid(0));
		cloaknpc("Iwin Recon#i19ms13",false,getcharid(0));
		cloaknpc("Iwin Recon#i19ms14",false,getcharid(0));
	}
end;
}

jor_que,271,88,5	script	Iwin Recon#i19ms11	21529,{
	if(isbegin_quest(17644) == 1){
		mes "[Iwin]";
		mes "Welcome!!";
		next;
		mes "[Horuru]";
		mes "What's the situation?";
		next;
		mes "[Iwin]";
		mes "The higher grade Rgans moved somewhere and only the lesser and intermediate-grades remained, making it somewhat chaotic.";
		next;
		cloaknpc("Iwin Recon#i19ms15",false,getcharid(0));
		cloaknpc("Human#i19ms91",false,getcharid(0));
		cloaknpc("Human#i19ms92",false,getcharid(0));
		cloaknpc("Human#i19ms93",false,getcharid(0));
		mes "[Escaping Human]";
		mes "Let me go! Let go!";
		npctalk "Why are you so strong?","Human#i19ms91",bc_self;
		next;
		mes "[Horuru]";
		mes "What are they?";
		next;
		mes "[Iwin]";
		mes "They're humans trying to escape, they looked dangerous that's why I brought them here.";
		npctalk "This is why I wished that I left much earlier!","Human#i19ms92",bc_self;
		next;
		mes "[Escaping Human]";
		mes "I'm not trying to escape, I was, going to take a toilet break!";
		next;
		mes "[Escaping Human]";
		mes "We really don't know anything! Everyone disappeared all of a sudden!";
		npctalk "Didn't you said that we'd wait and see??","Human#i19ms93",bc_self;
		next;
		mes "[Horuru]";
		mes "Lead those humans outside.";
		npctalk "Yes!","Iwin Recon#i19ms15",bc_self;
		npctalk "Where are you taking us!","Human#i19ms91",bc_self;
		npctalk "Let me go! Let go!!","Human#i19ms93",bc_self;
		next;
		mes "[Horuru]";
		mes "Did you see any more humans beside those people?";
		cloaknpc("Iwin Recon#i19ms15",true,getcharid(0));
		cloaknpc("Human#i19ms91",true,getcharid(0));
		cloaknpc("Human#i19ms92",true,getcharid(0));
		cloaknpc("Human#i19ms93",true,getcharid(0));
		next;
		mes "[Iwin]";
		mes "Some of them were wandering, so guided them outside. Some of them sem to have escaped already before we even entered.";
		next;
		mes "[Horuru]";
		mes "Where did all the Rgans move?";
		next;
		mes "[Iwin]";
		mes "We haven't found out yet!";
		next;
		mes "[Miriam]";
		mes "It's definitely strange. From what I've seen so far, I don7t think there was a thing where they would gathered separetely even though there was differences in their grades.";
		next;
		mes "[Lehar]";
		mes "The humans who were working here is also worried. I don't think they would have escaped safely in a chaotic situation.";
		next;
		mes "[Lazy]";
		mes "Then, we'd better take a look at the situation separately. Those you found during the search sent them all outside.";
		next;
		mes "[Horuru]";
		mes "One should stay here, and the rest should scatter and investigate the situation of this place.";
		npctalk "Yes!","Iwin Recon#i19ms11",bc_self;
		npctalk "Yes!","Iwin Recon#i19ms12",bc_self;
		npctalk "Yes!","Iwin Recon#i19ms13",bc_self;
		npctalk "Yes!","Iwin Recon#i19ms14",bc_self;
		next;
		cloaknpc("Horuru#i19ms11",true,getcharid(0));
		cloaknpc("Iwin Recon#i19ms12",true,getcharid(0));
		cloaknpc("Iwin Recon#i19ms13",true,getcharid(0));
		cloaknpc("Iwin Recon#i19ms14",true,getcharid(0));
		mes "[Lehar]";
		mes "Let's go to the place where they brought us before first. There might still be some humans there.";
		completequest 17644;
		setquest 17645;
		questinfo_refresh;
		getitem 1000608,25;
		close2;
		cloaknpc("Miriam#i19ms11",true,getcharid(0));
		cloaknpc("Lehar#i19ms11",true,getcharid(0));
		cloaknpc("Lazy#i19ms11",true,getcharid(0));
		end;
	}
	mes "[Iwin]";
	mes "I think we need to take a closer look at the situation in this place! I saw a trail in there!";
	close;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17644) == 1";
end;
}

jor_que,268,87,5	duplicate(dummynpc2)	Iwin Recon#i19ms12	21529
jor_que,276,90,3	duplicate(dummynpc2)	Iwin Recon#i19ms13	21529
jor_que,277,86,1	duplicate(dummynpc2)	Iwin Recon#i19ms14	21529
jor_que,271,84,7	duplicate(dummynpc2)	Iwin Recon#i19ms15	21529
jor_que,270,83,7	duplicate(dummynpc2)	Human#i19ms91	4_F_BRZ_WOMAN
jor_que,269,83,7	duplicate(dummynpc2)	Human#i19ms92	4_M_PHILMAN
jor_que,268,84,7	duplicate(dummynpc2)	Human#i19ms93	4_F_CAPEGIRL

jor_que,273,85,1	script	Horuru#i19ms11	21529,{
	npctalk "Let's hear what they have to say.","Horuru#i19ms11",bc_self;
	end;
}

jor_que,273,89,3	script	Miriam#i19ms11	21529,{
	npctalk "There's something wrong. Why is there only intermediate Rgans?","Miriam#i19ms11",bc_self;
	end;
}

jor_que,274,86,1	script	Lehar#i19ms11	21529,{
	npctalk "I can't get used to this place no matter how long I look.","Lehar#i19ms11",bc_self;
	end;
}

jor_que,270,86,7	script	Lazy#i19ms11	4_EP19_RGAN_SR3,{
	npctalk "if I ever I'm in danger, you have to protect me adventurer.","Lazy#i19ms11",bc_self;
	end;
}

jor_que,144,253,0	script	#i19ms21	HIDDEN_WARP_NPC,5,5,{
	end;
	
OnTouch:
	if(isbegin_quest(17645) == 1){
		cloaknpc("Lazy#i19ms12",false,getcharid(0));
		cloaknpc("Miriam#i19ms12",false,getcharid(0));
		cloaknpc("Human#i19ms95",false,getcharid(0));
		cloaknpc("Human#i19ms96",false,getcharid(0));
	}
end;
}

jor_que,144,253,3	script	Miriam#i19ms12	21529,{
	if(isbegin_quest(17645) == 1){
		mes "[Miriam]";
		mes "Just as I thought, they left behind all the humans";
		next;
		cloaknpc("Lehar#i19ms12",false,getcharid(0));
		npctalk "I, checked-","Lehar#i19ms12",bc_self;
		next;
		mes "[Lazy]";
		mes "How about elsewhere?";
		next;
		mes "[Lehar]";
		mes "The same goes for other places. They just left without doing anything. Did they move in a hurry?";
		npctalk "I handed them to the Iwin who passed by.","Lehar#i19ms12",bc_self;
		next;
		mes "[Lazy]";
		mes "Now~ Everyone, let's get out of here.";
		next;
		emotion ET_CONFUSE,getnpcid(0,"Human#i19ms94");
		mes "[Human Laborer]";
		mes "No! It's, a priest and a bishop!";
		next;
		mes "[Human Laborer]";
		mes "We didn't leave our place and stayed quiet!";
		npctalk "It's true!","Human#i19ms95",bc_self;
		next;
		mes "[Human Laborer]";
		mes "I'm telling you the truth! We all stayed quiet when everyone left!";
		npctalk "Don't punish me please...","Human#i19ms96",bc_self;
		next;
		mes "[Lehar]";
		mes "Don't worry, we're not here to hurt you.";
		next;
		mes "[Human Laborer]";
		mes "What? I know that face, we used to work here together, right...?";
		next;
		mes "[Lehar]";
		mes "Yes, Gregan is me. Do you understand now?";
		next;
		mes "[Human Laborer]";
		mes "Where did the Rgans who were guarding the entrance go?";
		next;
		mes "[Lazy]";
		mes "That's actually what we want to ask you.";
		next;
		mes "[Human Laborer]";
		mes "Well, the entrance was suddenly noisy, where did everyone go?";
		next;
		mes "[Human Laborer]";
		mes "Are they coming back?";
		next;
		mes "[Lazy]";
		mes "I don't know if they'll be back.";
		next;
		mes "[Miriam]";
		mes "Why are you still here? All the guards are gone.";
		next;
		mes "[Human Laborer]";
		mes "Oh, I don't know when they'll return...";
		next;
		mes "[Human Laborer]";
		mes "If you get caught escaping, you'll be put in a dire situation.";
		next;
		mes "[Human Laborer]";
		mes "And there are scary Rgans walking around outside. So we thought it would be better to wait here.";
		next;
		mes "[Miriam]";
		mes "I'll walk you to the entrance.";
		next;
		mes "[Lazy]";
		mes "Everyone, did you hear that, just follow her. You'll be safe.";
		next;
		mes "[Human Laborer]";
		mes "Why is a Rgan helping us?";
		next;
		mes "[Lehar]";
		mes "Oh, about that! Actually, we're humans just like you. So don't worry.";
		specialeffect EF_FOOD02,AREA,"Lehar#i19ms12";
		cloaknpc("Lehar#i19ms12",true,getcharid(0));
		cloaknpc("Lehar#i19ms13",false,getcharid(0));
		npctalk "Now, look.","Lehar#i19ms12",bc_self;
		next;
		mes "[Human Laborer]";
		mes "Oh?? Am I imagining things?";
		npctalk "Are you humans pretending as a Rgan? or a Rgan pretending as a human?","Human#i19ms96",bc_self;
		npctalk "This, what is going on?","Human#i19ms94",bc_self;
		next;
		mes "[Lehar]";
		mes "Will you believe us now? We're doing thos for various reasons, but we don't have the time to explain it now, so please just listen and follow her.";
		next;
		specialeffect EF_CHAINCOMBO,AREA,"Lehar#i19ms12";
		cloaknpc("Lehar#i19ms13",true,getcharid(0));
		cloaknpc("Lehar#i19ms12",false,getcharid(0));
		mes "[Human Laborer]";
		mes "Can we really go?";
		npctalk "Of course~","Lehar#i19ms12",bc_self;
		npctalk "Really, really??","Human#i19ms96",bc_self;
		next;
		mes "[Human Laborer]";
		mes "Oh, Thank you, thank you.";
		npctalk "I'm telling you the truth. Have you been deceived by them every single day?","Lazy#i19ms12",bc_self;
		npctalk "How do you know that?","Human#i19ms95",bc_self;
		next;
		mes "[Lazy]";
		mes "I'll go and find Lasgand, so you can go finish looking around here..";
		next;
		cloaknpc("Lazy#i19ms12",true,getcharid(0));
		mes "[Miriam]";
		mes "Then, I'll walk them off and join you back right away. Now, Shall we go?";
		npctalk "Oh~ Can I really go out...?","Human#i19ms94",bc_self;
		npctalk "Let's think about it as we go.","Human#i19ms96",bc_self;
		completequest 17645;
		setquest 17646;
		questinfo_refresh;
		close2;
		cloaknpc("Miriam#i19ms12",true,getcharid(0));
		cloaknpc("Lehar#i19ms12",true,getcharid(0));
		cloaknpc("Human#i19ms94",true,getcharid(0));
		cloaknpc("Human#i19ms95",true,getcharid(0));
		cloaknpc("Human#i19ms96",true,getcharid(0));
		end;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17645) == 1";
end;
}

jor_que,134,257,5	duplicate(dummynpc2)	Human#i19ms94	4_F_04
jor_que,136,254,5	duplicate(dummynpc2)	Human#i19ms95	4_M_LGTPOOR
jor_que,140,253,5	duplicate(dummynpc2)	Human#i19ms96	4_M_02
jor_que,139,257,5	duplicate(dummynpc2)	Lazy#i19ms12	4_EP19_RGAN_SR3
jor_que,146,252,3	duplicate(dummynpc2)	Lehar#i19ms12	21529
jor_que,146,252,3	duplicate(dummynpc2)	Lehar#i19ms13	4_EP19_LEHAR


jor_que,131,148,3	script	Intermediate Rgan#i19ms21	21529,5,5,{
	if(isbegin_quest(17646) == 1){
		cloaknpc("Horuru#i19ms31",false,getcharid(0));
		cloaknpc("Iwin#i19ms31",false,getcharid(0));
		cloaknpc("Iwin#i19ms32",false,getcharid(0));
		cloaknpc("Iwin#i19ms33",false,getcharid(0));
		mes "[Iwin]";
		mes "Atleast say something!";
		next;
		mes "[Intermediate Rgan]";
		mes "Have to work.";
		next;
		mes "[Horuru]";
		mes "Where did everyone go? I mean the senior Rgans.";
		next;
		mes "[Intermediate Rgan]";
		mes "You have to work too.";
		next;
		mes "[Iwin]";
		mes "Argh. The priests, the Bishops! Where did everyone go.";
		next;
		mes "[Intermediate Rgan]";
		mes "We have to work.";
		next;
		mes "[Iwin]";
		mes "Yeah, I'll work, so tell me. What did they say and where did they go.";
		next;
		mes "[Intermediate Rgan]";
		mes "Not allowed to play. Have to work.";
		next;
		mes "[Horuru]";
		mes "This is just, terrible.";
		npctalk "They're literally a work worms, working worms.","Iwin#i19ms32",bc_self;
		next;
		select("You were here.");
		mes "[Horuru]";
		mes "Did you find anything? I've searched while on the way here, but there was no higher-grade Rgans left here.";
		next;
		mes "[Horuru]";
		mes "All that was left are those under Intermediate Rgan, but they left the Intermediate Rgans who can't speak properly, ti doesn't make sense at all.";
		next;
		mes "[Intermediate Rgan]";
		mes "Great, great one...";
		next;
		mes "[Iwin]";
		mes "Oh, now he is talking about something other than work!!!";
		next;
		mes "[Horuru]";
		mes "Huh! Would like to try talking to an adventurer? When we try talking to them, they only talk about work, sigh.";
		next;
		select("Where did the priests and the bishops go?");
		mes "[Intermediate Rgan]";
		mes "Those.. great.. went gather... not us...";
		next;
		select("Gather?");
		mes "[Intermediate Rgan]";
		mes "To... the... lord...";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "So you're saying that the seniors and above gathered together on the order of Lasgand? Where is it?";
		npctalk "You understood that!","Iwin#i19ms31",bc_self;
		next;
		mes "[Intermediate Rgan]";
		mes "Yes, yes... upstairs... so, go... great one, you have to go too.";
		next;
		mes "[Horuru]";
		mes "It's up there, the higher level. Two of you follow me, the other one remains and continue to rescue the escapees.";
		npctalk "Yes!!","Iwin#i19ms31",bc_self;
		npctalk "Yes!!","Iwin#i19ms32",bc_self;
		npctalk "Yes!!","Iwin#i19ms33",bc_self;
		completequest 17646;
		setquest 17647;
		questinfo_refresh;
		close2;
		cloaknpc("Horuru#i19ms31",true,getcharid(0));
		cloaknpc("Iwin#i19ms31",true,getcharid(0));
		cloaknpc("Iwin#i19ms32",true,getcharid(0));
		cloaknpc("Iwin#i19ms33",true,getcharid(0));
		end;
	}
	if(isbegin_quest(17647) == 1){
		mes "[Rgan]";
		mes "Yes, yes... upstairs... so, go... great one, you have to go too.";
		close;
	}
	mes "[Rgan]";
	mes "Where am I...";
	close;
	
OnTouch:
	if(isbegin_quest(17646) == 1){
		cloaknpc("Horuru#i19ms31",false,getcharid(0));
		cloaknpc("Iwin#i19ms31",false,getcharid(0));
		cloaknpc("Iwin#i19ms32",false,getcharid(0));
		cloaknpc("Iwin#i19ms33",false,getcharid(0));
	}
end;

OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17646) == 1";
end;
}

jor_que,128,146,7	duplicate(dummynpc2)	Horuru#i19ms31	21529
jor_que,134,146,1	duplicate(dummynpc2)	Iwin#i19ms31	21529
jor_que,127,151,5	duplicate(dummynpc2)	Iwin#i19ms32	21529
jor_que,134,151,3	duplicate(dummynpc2)	Iwin#i19ms33	21529

jor_que,36,38,3	script	Rgan#i19ms20	21529,5,5,{
	npctalk "No, please don't hurt me...","Rgan#i19ms20",bc_self;
	end;
	
OnTouch:
	if(isbegin_quest(17647) == 1){
		cloaknpc("Lehar#i19ms21",false,getcharid(0));
		cloaknpc("Lazy#i19ms21",false,getcharid(0));
	}
	if(isbegin_quest(17648) == 1)
		cloaknpc("Lehar#i19ms21",false,getcharid(0));
end;
}


jor_que,34,38,5	script	Lazy#i19ms21	4_EP19_RGAN_SR3,{
	mes "[Lazy]";
	mes "I'll take a closer look at the situation here. There are more things that I can do here than there.";
	close;
}


jor_que,32,37,5	script	Lehar#i19ms21	21529,{
	if(isbegin_quest(17647) == 1){
		mes "[Lehar]";
		mes "You came just in time!!!";
		next;
		select("What happened?");
		cloaknpc("Aurelie#i19ms21",false,getcharid(0));
		npctalk "There you are.","Aurelie#i19ms21",bc_self;
		mes "[Lazy]";
		mes "The situation here is pretty bad.";
		next;
		mes "[Rgan]";
		mes "Don't.. hurt me.. please...";
		next;
		mes "[Lazy]";
		mes "See?";
		next;
		select("What's this? Why is the entrance blocked?");
		mes "[Aurelie]";
		mes "Didn't you sy that all intermediate Rgan above moved to the upper level?";
		next;
		mes "[Lazy]";
		mes "I know, right? That is also what I've heard.";
		mes "I was about to move to join the meeting. But there was something blocking it.";
		next;
		mes "[Lehar]";
		mes "That's right. These Rgans were cleaning up after the senior Rgans moved to the upper level in a hurry, but there was a senior Rgan attack from somewhere.";
		next;
		mes "[Aurelie]";
		mes "What? Suddenly?";
		next;
		mes "[Lehar]";
		mes "From what I heard, it didn't seem like the usual Senior Rgan.";
		next;
		select("I feel like I know this");
		npctalk "Right?","Lazy#i19ms21",bc_self;
		mes "[Lehar]";
		mes "That's why the senior Rgans blocked the entrance.";
		next;
		mes "[Aurelie]";
		mes "So, Bagot, Lasgand, and a bunch of senior Rgans are all in there?";
		next;
		mes "[Lehar]";
		mes "I think so. We need to go through this first to be able to do something.";
		next;
		select("What are you trying to do?");
		mes "[Lazy]";
		mes "Maybe, it's something that we can't talk about in front of little newphew?";
		next;
		mes "[Aurelie]";
		mes "If we use this well... we can leave them in one place and deal with them all at once.";
		next;
		cloaknpc("Horuru#i19ms21",false,getcharid(0));
		mes "[Horuru]";
		mes "That's why I called support.";
		next;
		cloaknpc("Voglinde#i19ms21",false,getcharid(0));
		cloaknpc("Leon#i19ms21",false,getcharid(0));
		sleep2 500;
		npctalk "How's the situation?","Voglinde#i19ms21",bc_self;
		npctalk "You haven't started yet, right?","Leon#i19ms21",bc_self;
		sleep2 1500;
		cloaknpc("Miriam#i19ms21",false,getcharid(0));
		sleep2 500;
		npctalk "Am I late?","Miriam#i19ms21",bc_self;
		sleep2 1500;
		cloaknpc("Iwin#i19ms21",false,getcharid(0));
		cloaknpc("Iwin#i19ms22",false,getcharid(0));
		sleep2 500;
		npctalk "The Advance Team! Have arrived!","Iwin#i19ms21",bc_self;
		npctalk "The Advance Team! Have arrived!","Iwin#i19ms22",bc_self;
		mes "[Lazy]";
		mes "Come, join the line. I'll briefly summarize everything.";
		next;
		mes "[Lazy]";
		mes "Lasgand, Bagot, and higher-grade Rgans are gathered beyond this wall on Lasgand's order.";
		mes "Did you understand everything up to there?";
		next;
		mes "[Lazy]";
		mes "And by the way. Beyond this wall is the residence of the senior Rgans.";
		next;
		mes "[Lazy]";
		mes "However, it seems that the Illuions did some modifications to the high ranking Rgans and then released them.";
		next;
		mes "[Voglinde]";
		mes "I'm sure that place is congested, but their strategy won't be that detailed.";
		mes "So you're saying we have to get rid of Rgans to get to meet Lasgand, right?";
		next;
		mes "[Horuru]";
		mes "Let's tear down the entrance and enter in small groups. The Iwins will take the lead.";
		next;
		mes "[Lehar]";
		mes "Are you ready? Okay, let's go in now.";
		npctalk "I'll meet you inside.","Horuru#i19ms21",bc_self;
		next;
		cloaknpc("Horuru#i19ms21",true,getcharid(0));
		cloaknpc("Iwin#i19ms21",true,getcharid(0));
		cloaknpc("Iwin#i19ms22",true,getcharid(0));
		mes "[Aurelie]";
		mes "It's our turn now. Leon, let's go.";
		next;
		cloaknpc("Leon#i19ms21",true,getcharid(0));
		cloaknpc("Aurelie#i19ms21",true,getcharid(0));
		mes "[Voglinde]";
		mes "Lehar, you should stay here and watch the situation. Miriam, shall we go?";
		next;
		npctalk "Don't worry. I'll be with you.","Lazy#i19ms21",bc_self;
		cloaknpc("Voglinde#i19ms21",true,getcharid(0));
		cloaknpc("Miriam#i19ms21",true,getcharid(0));
		mes "[Lazy]";
		mes "Now, it's the adventurers turn. Be ready.";
		completequest 17647;
		setquest 17648;
		questinfo_refresh;
		close;
	}
	if(isbegin_quest(17648) == 1){
		.@party_id = getcharid(1);
		if(!.@party_id){
			mes "[Lehar]";
			mes "You haven't joined a party. Join or form a aprty for one or more person. We have to mark who is who.";
			close;
		}
		mes "[Lehar]";
		mes "Are you ready? When you're ready, I'll send the signal to let the people inside know that you're going in.";
		next;
		.@md_name$ = "Chaotic Snake Nest";
		switch(select("Yes, send a signal.:I'll go in.:I need to prepare.")){
			case 1:
				if(instance_create(.@md_name$) == -3) instance_warning(2,.@md_name$);
				mes "[Lehar]";
				mes "I'll open the entrance as soon as I receive the signal. Please wait a moment.";
				close;
			
			case 2:
				if(instance_enter(.@md_name$) == IE_OK){
					mes "[Lehar]";
					mes "I just got the signal that you can enter! You can go in!";
					close2;
					mapannounce "jor_que", strcharinfo(0) + " of the party, "+ getpartyname(.@party_id) +", is entering the " + .@md_name$ + ".", bc_map, "0x00FF99";
				} else {
					mes "[Lehar]";
					mes "They'll send a signal when it's ready. The strategy could go awry inside.";
					close;
				}
				end;
				
			case 3:
				mes "[Lehar]";
				mes "Aren't you ready yet? I'll be waiting for you.";
				close;
		}
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17647) == 1";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17648) == 1";
end;
}

jor_que,37,36,1	duplicate(dummynpc2)	Horuru#i19ms21	21529
jor_que,38,33,1	duplicate(dummynpc2)	Iwin#i19ms21	21529
jor_que,37,31,1	duplicate(dummynpc2)	Iwin#i19ms22	21529
jor_que,36,35,1	duplicate(dummynpc2)	Aurelie#i19ms21	4_EP19_AURELIE
jor_que,35,34,1	duplicate(dummynpc2)	Leon#i19ms21	4_EP19_LEON
jor_que,34,32,7	duplicate(dummynpc2)	Miriam#i19ms21	21529
jor_que,40,34,1	duplicate(dummynpc2)	Voglinde#i19ms21	21529

jor_nest,22,255,6	script	Rope#whl	4_ROPEPILE,{
	if(isbegin_quest(17637) == 0){
		mes "^0000FFThere is a rope that appears to have been used to secure the airship.^000000";
		close;
	}
	.@party_id = getcharid(1);
	removespecialeffect EF_ITEM_LIGHT,AREA,"#whl_effect";
	specialeffect EF_ITEM_LIGHT,AREA,"#whl_effect";
	if(!.@party_id){
		mes "^0000FFPlease organize a party of 1 or more to proceed.^000000";
		close;
	}
	if(isbegin_quest(17637) == 1){
		select("Open the note that lazy gaved near the rope");
		mes "^0000FFThere's a lot of messy writing and covered with bill notes^000000";
		next;
		while(select("Take a closer look at the back.:Throw it away since it looks useless.") == 2){
			mes "There are hints that are not too obvious, I shouldn't throw it away.";
			next;
		}
		cutin "ep19_leizi02.png",2;
		mes "[Lazy]";
		mes "Oh, looks like you found the important phrases. I hope you understand it, I didn't have the time to write it in detail.";
		next;
		cutin "ep19_leizi03.png",2;
		mes "[Lazy]";
		mes "From this point. It says the airship that the Illusions are using are the same as ours. We can't just break it.";
		next;
		select("Try combining the sentences between the scribble.");
		cutin "ep19_leizi02.png",2;
		mes "[Lazy]";
		mes "Ah, that's Ginger's advice. It's the person who moves and control the airship.";
		next;
		select("Examine the parts with discoloration.");
		mes "[Lazy]";
		mes "Let's see. According to Ginger's analysis report, the energy core is an artifical intelligence that can repair itself indefinitely. She said that this energy core is very intelligent.";
		next;
		cutin "ep19_leizi03.png",2;
		mes "[Lazy]";
		mes "Do you understand? The core of this airship is like a bomb that's capable of generating infinite energy.";
		next;
		select("Look under the lunch bill total.");
		cutin "ep19_leizi02.png",2;
		mes "[Lazy]";
		mes "If we simply destroy it. In the minimum, it will destroy everything in the 10km radius around it! It's fantastic!";
		next;
		select("Look at the last unfolded part.");
		cutin "ep19_leizi03.png",2;
		mes "[Lazy]";
		mes "When it accumulate a certain level of damage, the core will stop first. If it accumulate more damage in that state, it will create a huge explosion which looks like a defect.";
		next;
		cutin "ep19_leizi02.png",2;
		mes "[Lazy]";
		mes "Varmundt probably didn't think that an AI would be an active bomb. I guess that's the reason that he never produced an additional model of it.";
		next;
		cutin "ep19_leizi03.png",2;
		mes "[Lazy]";
		mes "Ginger said that, ^0000ff[this was master's only sore finger. I didn't quite understand what she meant]^000000.";
		next;
		mes "[Lazy]";
		mes "It's difficult to break, and we are certain that it will restore itself if it were destroyed. We have no other choice. To stir the inside to an extent that the aircraft can't operate and swish! We'll run away after.";
		next;
		cutin "ep19_leizi02.png",2;
		mes "[Lazy]";
		mes "I can see your expression right now. Anyway, I gave that advice to the old woman because I thought she would seriously break the airship, so thank me!";
		next;
		cutin "ep19_leizi01.png",2;
		mes "[Lazy]";
		mes "Then, I wish you goodluck! I'll see you again if I can!";
		close2;
		cutin "",255;
		completequest 17637;
		end;
	}
	switch(checkquest(12560,PLAYTIME)){
		case -1:
			break;
			
		case 0:
		case 1:
			end;
			
		case 2:
			erasequest 12560;
			break;
	}
	.@md_name$ = "Airship Sabotage Operation";
	.@name$ = instance_live_info(ILI_NAME,instance_id(IM_PARTY));
	if(.@name$ != .@md_name$){
		mes "^0000FFIt's a rope with a hook. It seems that they used it to ride the airship.^000000";
		next;
		if(select("Cancel.",(is_party_leader() ? "Attach the rope on the airship.":"")) == 1)
			end;
		if(instance_create(.@md_name$) == -3) 
			instance_warning(2,.@md_name$);
	} else {
		mes "The rope seems to be tightly attached on the airship.";
		next;
		select("Use the rope to enter.");
		if(instance_enter(.@md_name$) == IE_OK){
			//No enter announcement for some reason
			//mapannounce "jor_nest", strcharinfo(0) + " of the party, "+ getpartyname(.@party_id) +", is entering the " + .@md_name$ + ".", bc_map, "0x00FF99";
			setquest 12560;
			if(!isbegin_quest(12561)) setquest 12561;
		} else
			mes "- ^0000FFThe memorial dungeon has not been created yet. Please check again.^000000 -";
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17637) == 1";
end;
}

jor_nest,21,258,1	duplicate(dummynpc2)	#whl_effect	CLEAR_NPC

jor_tail,221,57,3	script	Iwin Soldier#ep19iwin01	21515,{
	if(isbegin_quest(18125) == 1){
		cutin "ep19_iwin11.png",2;
		mes "[Iwin Soldier]";
		mes "The road is long and a bit complicated.";
		mes "Follow the sign that I'll put for you.";
		close2;
		cutin "",255;
		navigateto("icas_in",141,216);
		end;
	}
	cutin "ep19_iwin11.png",2;
	mes "[Iwin Soldier]";
	mes "Don't worry about this place, you can go.";
	mes "We will be watching the airship.";
	close3;
}

jor_tail,223,54,3	script	Iwin Soldier#ep19iwin02	21515,{
	cutin "ep19_iwin04.png",2;
	mes "[Iwin Soldier]";
	mes "Oh no, you don't have any feathers.";
	mes "Aren't you cold?";
	close3;	
}

jor_tail,239,40,5	script	Goriri#ep19in	21515,{
	mes "[Goriri]";
	mes "Everything on duty is clear!";
	close;
}

jor_tail,285,94,5	script	Doraro#ep19in	21515,{
	mes "[Doraro]";
	mes "I'm on duty... hm? I haven't seen you before.";
	mes "Be careful not to catch a cold.";
	mes "You don't even have a single feather... *Tweets*.";
	close;
}

jor_tail,291,91,5	script	Hamour#e19c00	4_EP19_IWIN,{
	npctalk "Is this the end of the sea?","",bc_self;
	end;
}

jor_tail,209,72,5	script	Security#e19c02	21517,{
	npctalk "I'm on guard! You're clear! Welcome to Isgard!";
	end;
}

jor_tail,216,79,3	script	Lochho#e19c00	4_EP19_IWIN,{
	npctalk "Do you know what happen after you clear the snow? It will snow again! Wahahahahahaha!!!","",bc_self;
	end;
}

jor_tail,168,86,3	script	Goruru#e19c00	4_EP19_IWIN,{
	npctalk "Have you ever seen someone buried in snow? I'm testing the feather agent I developed, it's waterproof and has cold protection.","",bc_self;
	end;
}

jor_tail,150,81,5	script	Bazaar#e19c00	4_EP19_IWIN,{
	npctalk "Well, if you're not resting, why aren't you working?","",bc_self;
	end;
}

jor_tail,153,76,3	script	Nodor#e19c00	4_EP19_IWIN,{
	npctalk "I hope this basket will be full.","",bc_self;
	end;
}

jor_tail,181,158,5	script	Hoheho#e19c00	4_EP19_IWIN,{
	npctalk "Can you see the footprints up there? It's a sacred footprint. It's all over the place, so look at where you're going carefully.","",bc_self;
	end;
}

jor_tail,56,196,3	script	Poryaryo#ep19in	21515,{
	mes "[Poryaryo]";
	mes "Shh... Be quiet.";
	mes "Do not disturb with my fishing.";
	mes "Go to another fishing spot.";
	close;
}

jor_tail,216,269,5	script	Haryara#e19c00	4_EP19_IWIN,{
	npctalk "Be careful, the road is narrow. It will take a long time to rescue you if it collapses. ... Don't you believe me?","",bc_self;
	end;
}

jor_tail,250,273,7	script	Jororon#e19c00	4_EP19_IWIN,{
	npctalk "There's a piece of ice behind the Ice Castle, and one of it has something that looks exactly like this on it's head. It's strange.","",bc_self;
	end;
}

//JOR BACK
jor_back1,376,247,5	script	Watcher#ep19in	21515,{
	mes "[Watcher]";
	mes "Everything on duty is clear... *Sneeze*.";
	close;
}

//= Ice Castle
icecastle,25,113,4	script	Announcement#iwp_1	4_BOARD3,{
	mes "[Announcement]";
	mes "「Eagle Patrol」are recruiting responsible adventurers to join us on patrols.";
	next;
	mes "[Qualification Requirements]";
	mes "All races aside from Rgans that are physically healthy and able to communicate.";
	next;
	mes "[Application Inquiries]";
	mes "<Eagle Patrol>";
	mes "<NAVI>[Captain Marsha Gigi Happy Rev Eve<INFO>icecastle,23,115,0,101,0</INFO></NAVI>";
	mes "<NAVI>Alice Lloyd Broad-Minded<INFO>icecastle,23,115,0,101,0</INFO></NAVI>";
	mes "<NAVI>Tess Vortex]<INFO>icecastle,23,115,0,101,0</INFO></NAVI>";
	mes "We're hoping for your support.";
	close;
}

icecastle,43,104,5	script	Choruryo#1	4_EP19_IWIN,{
	donpcevent strnpcinfo(0) + "::OnEvent";
	mes "[Choruryo]";
	mes "Are you an adult human?";
	mes "Adults do the real patrols. Don't be fooled by what we're doing.";
	close;
	
OnEvent:
	npctalk "Jororo : I'm disappointed in you guys!","Jororo#1";
	sleep 1000;
	npctalk "Nororu : I'm blinded!","Nororu#1";
	sleep 1000;
	npctalk "Doryaro : Order!","Doryaro#1";
	sleep 1000;
	npctalk "Choruryo : It's a new order!","Choruryo#1";
end;
}

icecastle,45,102,5	script	Doryaro#1	4_EP19_IWIN,{
	donpcevent strnpcinfo(0) + "::OnEvent";
	mes "[Doryaro]";
	mes "Are you an adult human?";
	mes "Adults do the real patrols. Don't interfere with our training.";
	close;
	
OnEvent:
	npctalk "Jororo : I'm disappointed in you guys!","Jororo#1";
	sleep 1000;
	npctalk "Nororu : I'm blinded!","Nororu#1";
	sleep 1000;
	npctalk "Doryaro : Order!","Doryaro#1";
	sleep 1000;
	npctalk "Choruryo : It's a new order!","Choruryo#1";
end;
}

icecastle,47,100,5	script	Nororu#1	4_EP19_IWIN,{
	donpcevent strnpcinfo(0) + "::OnEvent";
	mes "[Nororu]";
	mes "Are you an adult human?";
	mes "Adults do the real patrols. We're in charge of this yard.";
	close;

OnEvent:
	npctalk "Jororo : (Whispers)","Jororo#1";
	sleep 1000;
	npctalk "Nororu : (Tweets)","Nororu#1";
	sleep 1000;
	npctalk "Doryaro : What are you saying?","Doryaro#1";
	sleep 1000;
	npctalk "Choruryo : Let me in it too~","Choruryo#1";
end;
}

icecastle,49,98,5	script	Jororo#1	4_EP19_IWIN,{
	donpcevent strnpcinfo(0) + "::OnEvent";
	mes "[Jororo]";
	mes "Are you an adult human?";
	mes "Adults can do the real patrols, so we can't include you in our patrol game.";
	close;
	
OnEvent:
	npctalk "Jororo : I'm disappointed in you guys!","Jororo#1";
	sleep 1000;
	npctalk "Nororu : I'm blinded!","Nororu#1";
	sleep 1000;
	npctalk "Doryaro : Order!","Doryaro#1";
	sleep 1000;
	npctalk "Choruryo : It's a new order!","Choruryo#1";
end;
}

icecastle,55,124,3	script	Sharyara#ep19iwin06	21515,{
	if(isbegin_quest(17624) < 2){
		cutin "ep19_iwin04.png",2;
		mes "[Sharyara]";
		mes "Be careful of the ice?";
		mes "Is it really that slippery?~?";
		close3;
	}
	cutin "ep19_iwin04.png",2;
	mes "[Sharyara]";
	mes "It's hard to get into Rgan's nest and work these days.";
	mes "Are you going to infiltrate again today?";
	mes "It must be hard to walk on the icy road, so can I help you?";
	next;
	if(select("Can you bring me to the snake's nest?:It's alright.") == 2){
		mes "[Sharyara]";
		mes "Let me know if you're going right away.";
		mes "I'm the express deliverer of the Ice Castle.";
		mes "It's my job send a human being off.";
		close3;
	}
	mes "[Sharyara]";
	mes "Last you weren't you transformed into a Rgan, right?";
	mes "It was really fun. I remember it very well.";
	mes "I'll send it to you right at the entrance, so go in quickly. Okay?";
	close2;
	cutin "",255;
	warp "jor_nest",275,19;
	end;

OnInit:
	questinfo QTYPE_CLICKME,QMARK_YELLOW,"isbegin_quest(17624) == 2";
end;
}

icecastle,59,123,6	script	Koraryo#icecastle	21513,{
	mes "[Koraryo]";
	mes "Snow here, Ice there, it's everywhere";
	mes "It's an Ice Castle full of only snow and ice.";
	mes "Welcome, strangers!";
	mes "I'm ^B47096Silky Well-Balanced Thomas Joy^000000님이";
	mes "I'm in charge of guiding people here, ask me if you have any questions.";
	next;
	mes "[Koraryo]";
	mes "Click on the ^B9062F[name] of the location^000000 you're looking for, and I will guide you through the ^B9062Fnavigation^000000 system.";
	next;
	while(true){
		switch(select("[     Ice Castle     ]:[     Barracks     ]:[  Belgund Laboratory  ]:[     Restaurant     ]:[     Inn     ]:Clear Map Display:Quit.")){
			case 1:
				mes "[Koraryo]";
				mes "<NAVI>[Ice Castle]<INFO>icecastle,213,174,0,101,0</INFO></NAVI>";
				viewpoint 1,213,174,1,0xFF0A82FF;
				break;
				
			case 2:
				mes "[Koraryo]";
				mes "<NAVI>[Barracks]<INFO>icecastle,186,222,0,101,0</INFO></NAVI>";
				viewpoint 1,186,222,2,0xFFAAFF00;
				break;
				
			case 3:
				mes "[Koraryo]";
				mes "<NAVI>[벨군드 연구실]<INFO>icas_in,167,61,0,101,0</INFO></NAVI>";
				viewpoint 1,186,222,3,0xFF008080;
				break;
				
			case 4:
				mes "[Koraryo]";
				mes "<NAVI>[식당]<INFO>icecastle,124,171,0,101,0</INFO></NAVI>";
				viewpoint 1,124,171,4,0xFFFF1493;
				break;
				
			case 5:
				mes "[Koraryo]";
				mes "<NAVI>[Inn]<INFO>icecastle,64,224,0,101,0</INFO></NAVI>";
				viewpoint 1,64,224,5,0xFF8B4513;
				break;
				
			case 6:
				mes "[Koraryo]";
				mes "All the marks has been removed!";
				mes "Do you need any other help?";
				viewpoint 2,213,174,1,0xFFFFFFFF;
				viewpoint 2,186,222,2,0xFFFFFFFF;
				viewpoint 2,186,222,3,0xFFFFFFFF;
				viewpoint 2,124,171,4,0xFFFFFFFF;
				viewpoint 2,64,224,4,0xFFFFFFFF;
				next;
				continue;
				
			case 7:
				mes "[Koraryo]";
				mes "Outsiders must be cold since you have little hair, they go around from place to place for no reason, so stay in a warm building and don't catch a cold.";
				end;
		}
		mes "↑If you click this, I'll guide you. You might be wondering where you will go, so I'll mark it on your minimap too!";
		mes "Do you need information about other places?";
		next;
	}
	end;
}

icecastle,77,106,3	script	Mororyo#ep19in	4_EP19_IWIN,{
	mes "[Mororyo]";
	mes "The fish can always remain fresh, but it will depend on how you trim it.";
	mes "There's a technique that has been passed down from generation to generation.";
	mes "Do you want me to teach you?";
	next;
	if(select("Yes.:No.") == 2){
		mes "[Mororyo]";
		mes "That's a pity.";
		close;
	}
	mes "[Mororyo]";
	mes "Alright. Now, let's put your beak on the gills...";
	next;
	mes "[" + strcharinfo(0) + "]";
	mes "...";
	next;
	mes "[Mororyo]";
	mes "...";
	next;
	mes "[Mororyo]";
	mes "Humans don't have beaks.";
	mes "I overlooked that.";
	close;
}

icecastle,73,101,7	script	Joraran#ep19in	4_EP19_IWIN,{
	mes "It's watching Mororyo's skills with twinkling eyes.";
	npctalk "Mororyo : I'll teach you when you grow up.","Mororyo#ep19in",bc_self;
	npctalk "Joraran : Yes!","",bc_self;
	close;
}

icecastle,82,111,3	script	Shashasha#e19c00	4_EP19_IWIN,{
	npctalk "Did you know? Whoever pulls this spear out shall gain the world. My uncle did it before.","",bc_self;
	end;
}

icecastle,84,129,3	script	Banjiru#e19c00	4_EP19_IWIN,{
	npctalk "It's pretty, right? It's so pretty that I brought it out here for everyone to see.";
	end;
}

icecastle,85,95,5	script	Toriro#ep19in	4_EP19_IWIN,{
	mes "[Toriro]";
	mes "I won't give this to you!";
	mes "If you human want to eat too, go and grind the ice over there.";
	next;
	mes "[" + strcharinfo(0) + "]";
	mes "What?";
	next;
	mes "[Toriro]";
	mes "It's a snow slush!";
	mes "Grind the ice with you beak and sprinkle it with chopped calamari...";
	mes "Oh right...";
	next;
	mes "[Toriro]";
	mes "Humans don't have beaks.";
	mes "It must be uncomfortable.";
	npctalk "Porara : Oh... no beak...","Porara#ep19in",bc_self;
	npctalk "Oriru : Oh...","Oriru#ep19in",bc_self;
	npctalk "Nororyon : Hiccup.","Nororyon#ep19in",bc_self;
	close;
}

icecastle,86,100,5	script	Porara#ep19in	4_EP19_IWIN,{
	mes "[Porara]";
	mes "Human, do you drink alcohol?";
	mes "It will warm you up if you drink this.";
	mes "It's made by dipping edible seaweed stems.";
	next;
	mes "The smell of very strong alcohol is harmonious with the scent of the deep sea.";
	next;
	mes "[" + strcharinfo(0) + "]";
	mes "...I'll pass on it.";
	next;
	mes "[Porara]";
	mes "The seaweed is also delicious when eaten separately.";
	mes "That's a shame.";
	close;
}

icecastle,90,100,3	script	Nororyon#ep19in	4_EP19_IWIN,{
	mes "[Nororyon]";
	mes "Human, would you like to have a drink?";
	next;
	mes "[" + strcharinfo(0) + "]";
	mes "What is it?";
	next;
	mes "[Nororyon]";
	mes "It's Adkara. It will warm up your body.";
	next;
	mes "It smells like very strong alcohol.";
	mes "The Iwin's face that is hidden under it's fur looks like it's dyed with red.";
	next;
	mes "[" + strcharinfo(0) + "]";
	mes "No thanks.";
	next;
	mes "[Nororyon]";
	mes "It's delicious.";
	close;
}

icecastle,89,94,3	script	Oriru#ep19in	4_EP19_IWIN,{
	mes "[Oriru]";
	mes "Humans, if you're an adult then you can drink.";
	mes "You can't drink water outside. Go indoors and melt some ice.";
	close;
}

icecastle,97,177,4	script	Supplier#iws	21518,{
	if(191_scroll_check()){
		emotion ET_HUK,getnpcid(0,"Supplier#iws");
		npctalk "Supplier Goril : Oh! You scared me.","Supplier#iws",bc_self;
		sleep2 1000;
		npctalk "Supplier Goril : Huh? a Rgan here?","Supplier#iws",bc_self;
		sleep2 1000;
		emotion ET_THINK,getnpcid(0,"Supplier#iws");
		npctalk "Supplier Goril : ... ... ...","Supplier#iws",bc_self;
		sleep2 3000;
		emotion ET_SURPRISE,getnpcid(0,"Supplier#iws");
		npctalk "Supplier Goril : You're the agent who turns into a Rgan. Yeah, if you're an agent you should be able to do that much.","Supplier#iws",bc_self;
	}
	if(isbegin_quest(5972) < 2){
		emotion ET_FRET,getnpcid(0);
		mes "[Supplier Gorir]";
		mes "Are you going to bring foods into the Frozen Scale Hills to catch bugs?";
		next;
		mes "[Supplier Gorir]";
		mes "Huh? Who are you?";
		mes "Ah! Are you the adventurer who recently came in? I have my fierce eyes on you, so you'd better not do anything stupid.";
		close;
	}
	switch(checkquest(5977,PLAYTIME)){
		case -1:
			break;
		case 0:
		case 1:
			emotion ET_STARE_ABOUT,getnpcid(0,"Supplier#iws");
			mes "[Supplier Goril]";
			mes "Ahaha... the weather is quite warm today. Have a nice day, adventurer..";
			next;
			mes "The Supply Officer Goril is desperately trying to pretend he doesn't know me. Is there some kind of rule of ignoring people when there are no mission?";
			close;
		
		case 2:	
			erasequest 5977;
			emotion ET_SPARK,getnpcid(0,"Supplier#iws");
			npctalk "Supplier Goril : I have a hunch.. a hunch..","Supplier#iws",bc_self;
			break;
	}
	if(isbegin_quest(5979) == 1){
		emotion ET_QUESTION,getnpcid(0,"Supplier#iws");
		mes "[Supplier Goril]";
		mes "Did someone put something there? Yes agent! Of course there is, did you retrieve the item? Oh there it is. Then give it to me.";
		next;
		mes "[Supplier Goril]";
		mes "Is that the thing you have in your hand right now? Hmm... That's the food that came out for lunch yesterday. It's quite hard to identify who hid this.";
		next;
		erasequest 5979;
		if(countitem(528)) delitem 528,1;
		setquest 5977;
		addreputation("Ice Castle",5);
		getitem 1000608,2 + (getreputation("Ice Castle") >= 3000 ? 1 : 0);
		mes "[Supplier Goril]";
		mes "Agent, this is a reward for your work and dedication. Of course you wouldn't want something like this. I know it..";
		next;
		emotion ET_COOL,getnpcid(0,"Supplier#iws");
		mes "[Supplier Goril]";
		mes "But please think of it as my little heart to ask for tomorrow as well.";
		close;
	}
	if(isbegin_quest(5976) == 0){
		mes "[Supplier Goril]";
		mes "Recent investigations that an unidentified entity is bringing food to the wildlife at the Frozen Scale Hills.";
		next;
		mes "[Supplier Goril]";
		mes "If left unattended, it may cause several problems such as increasing amount of bugs.";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "Hello?";
		next;
		mes "[Supplier Goril]";
		mes "It may seem like a good behavior at first glance, but in long-term it's very dangerous for the ecosystem.";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "Over here...";
		next;
		mes "[Supplier Goril]";
		mes "Huh? There's an idle agent here as if it's waiting. Agent! I'll assign you a mission.";
		next;
 		mes "[Supplier Goril]";
		mes "Retrieve the food from the unidentified entity from the Frozen Scale Hills!";
		next;
		select("I'll do it.:I'd be happy to do it.:I'll do anything.:There's no other option, right?");
		emotion ET_STARE,getnpcid(0,"Supplier#iws");
		mes "[Supplier Goril]";
		mes "There is no such thing as rejection from a true agent.";
		next;
		setquest 5976;
		completequest 5976;
		setquest 5978;
		mes "[Supplier Goril]";
		mes "Agent! I'm always grateful for your dedication.";
		next;
		mes "[Supplier Goril]";
		mes "The retrieval mission is top secret.. You can do it while patrolling, or just passing by the Frozen Scale Hill..";
		next;
		mes "[Supplier Goril]";
		mes "I'll leave it entirely up to you when to handle it.";
		next;
		emotion ET_BEST,getnpcid(0,"Supplier#iws");
		mes "[Supplier Goril]";
		mes "Goodluck, agent!";
		close;
	} else {
		mes "[Supplier Goril]";
		mes "There was an information that someone had brought a food again. Will you be with us again this time, agent?";
		next;
		if(select("I'll be with you.:It's a fake information.") == 2){
			emotion ET_HUK,getnpcid(0,"Supplier#iws");
			mes "[Supplier Goril]";
			mes "False! False information? I almost made a big mistake. I'll check more and deliver the mission.";
			close;
		}
		setquest 5978;
		mes "[Supplier Goril]";
		mes "I'll leave everything to you again this time. Goodluck, agent!";
		close;
	}
	if(isbegin_quest(5978) == 1){
		mes "[Supplier Goril]";
		mes "Everything about this missiong is entirely up to you, agent.";
		next;
		mes "[Supplier Goril]";
		mes "Oh! If it's about advice, I'll give you one. It's obvious that they hid it even though they're acting that it's not!";
		next;
		emotion ET_BEST,getnpcid(0,"Supplier#iws");
		mes "[Supplier Goril]";
		mes "I'm sure they hid it in an obvious place since they thought nobody would find it. Goodluck, agent!";
		close;
	}
	end;
	
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(5972) == 2 && isbegin_quest(5976) == 0";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(5976) == 2 && isbegin_quest(5979) == 1";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(5976) == 2 && isbegin_quest(5978) == 0 && checkquest(5977,PLAYTIME) == 2";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(5976) == 2 && isbegin_quest(5978) == 0 && checkquest(5977,PLAYTIME) == -1";
	setunittitle getnpcid(0),"<Eagle Patrol>";
end;
}

jor_back1,113,142,0	script	Something#ff01	1127,{
	mes "「^4A94F7Something^000000」 seems to be slightly covered in dirt. I think it's moving, this is a sand, right?";
	next;
	if(select("Take a look.:Just leave it.") == 2){
		mes "Okay! A「something」 might come out, so I'll just leave it alone.";
		close;
	}
	progressbar "FFFFFF",1;
	.@rand = rand(1,100);
	if(.@rand < 80){
		mes "Ah.. is it really just a pile of dirt?";
		close2;
		if(isbegin_quest(5978)){
			hideonnpc;
			initnpctimer;
		}
	} else if(.@rand < 90){
		mes "Eek! It this bugs? Why is it coming out from here?";
		close2;
		if(isbegin_quest(5978)){
			hideonnpc;
			initnpctimer;
		}
		getmapxy(.@map$,.@x,.@y,BL_NPC);
		for(.@i = 0; .@i < 5; .@i++)
			monster .@map$,.@x,.@y,"Bug",1608,1,strnpcinfo(0) + "::OnBugKill";
	} else {
		if(isbegin_quest(5978) == 0){
			mes "Ah.. is it really just a pile of dirt?";
			close;
		} else {
			erasequest 5978;
			setquest 5979;
			getitem 528,1;
			mes "I found a food that they put here. Let's bring it to Supplier Goril.";
			close;
		}
	}
	end;
	
OnTimer30000:
	stopnpctimer;
	killmonster "jor_back1",strnpcinfo(0) + "::OnBugKill";
	hideoffnpc;
end;

OnBugKill:
end;
}

jor_back1,292,259,0	duplicate(Something#ff01)	Something#ff03	1127
jor_back1,265,154,0	duplicate(Something#ff01)	Something#ff04	1127
jor_back1,278,103,0	duplicate(Something#ff01)	Something#ff05	1127
jor_back1,152,60,0	duplicate(Something#ff01)	Something#ff06	1127
jor_back1,100,70,0	duplicate(Something#ff01)	Something#ff07	1127
jor_back1,136,175,0	duplicate(Something#ff01)	Something#ff08	1127
jor_back1,149,239,0	duplicate(Something#ff01)	Something#ff09	1127

icecastle,103,227,5	script	Joriru#ep19in	4_EP19_IWIN,{
	mes "[Joriru]";
	mes "What to do for dinner...";
	close;
}

icecastle,105,227,3	script	Torura#ep19in	4_EP19_IWIN,{
	mes "[Torura]";
	mes "I want to eat something different from yesterday...";
	close;
}

icecastle,118,245,5	script	Gyaruru#e19c00	21514,{
	npctalk "Stay still. I'm going to catch all the fish here today.";
	end;
}

icecastle,71,255,5	script	Ayaru#e19c00	4_EP19_IWIN,{
	npctalk "Uhuh- Stop!!! Be careful not to eraser it. It's a sacred footprint.";
	end;
}

icecastle,149,216,3	script	Borarya#ep19in	21516,{
	mes "[Boriri]";
	mes "All clear on duty!";
	close;
}

icecastle,180,206,3	script	Iwin Soldier#ep19iwin04	21515,{
	cutin "ep19_iwin09.png",2;
	mes "[Iwin Soldier]";
	mes "Well... No matter how much you say this is important...";
	mes "Is it meaningful to continue clearing this snow? Is there?";
	next;
	cutin "ep19_iwin10.png",2;
	mes "[Iwin Soldier]";
	mes "Oh... Isn't it good enough to clear it before it piles up?";
	mes "Yes, that's right.";
	mes "No.. but...";
	close3;
}

icecastle,190,217,3	script	Iwin Soldier#ep19iwin03	21515,{
	cutin "ep19_iwin04.png",2;
	mes "[Iwin Soldier]";
	mes "Do you have any business in the barracks?";
	mes "The room on the left of the barracks is the patrol unit room, and on the right is the Vellgunde Laboratory.";
	mes "I saw a man named Maram doing business.";
	mes "He says a lot of interesting things.";
	close3;
}

icecastle,248,211,1	script	Doluri#ep19in	4_EP19_IWIN,{
	mes "[Doluri]";
	mes "This work doesn't have the name of the animal on it.";
	mes "Is this what our ancestors looked like?";
	close;
}

icecastle,261,179,7	script	Goryuryu#ep19in	4_EP19_IWIN,{
	mes "[Goryuryu]";
	mes "Phew...";
	next;
	mes "[Goryuryu]";
	mes "I can't tell you how grateful I am for such a peaceful day without anything to do.";
	mes "Even this cold sea looks warm in this kind of time.";
	close;
}

icecastle,241,146,7	script	Chorari#ep19in	4_EP19_IWIN,{
	mes "[Chorari]";
	mes "There's no creature around here that looks like this.";
	mes "Did the real thing that our ancestors saw looked exactly like this?";
	close;
}

icecastle,195,170,3	script	Guard#e19c00	21517,{
	npctalk "On alert.","",bc_self;
	end;
}

icecastle,209,156,3	script	Guard#e19c01	21517,{
	npctalk "No problem here.","",bc_self;
	end;
}

icecastle,172,153,3	script	Noriri#ep19in	4_EP19_IWIN,{
	mes "It's looking at a very distant place with vague eyes.";
	mes "Whatever the case...";
	mes "There's nothing in sight.";
	close;
}

icecastle,133,165,5	script	Haruru#e19c00	4_EP19_IWIN,{
	npctalk "Is it pretty? I carefully put it together.","",bc_self;
	end;
}

icecastle,170,142,3	script	Chorirong#ep19in	4_EP19_IWIN,{
	mes "[Chorirong]";
	mes "Occasionally, there are friends who have injuries on their feather, it seems like another inflicted it while escaping.";
	next;
	mes "[Chorirong]";
	mes "Have you seen an animal that's not afraid a cage of that size?";
	mes "Is it the rumored cat or perhaps dog?";
	close;
}

icecastle,101,216,5	script	Goryuro#ep19in	4_EP19_IWIN,{
	mes "[Goryuro]";
	mes "Huh? There are many with similar names?";
	mes "Isn't it completely different?";
	mes "You people just don't understand.";
	close;
}

icecastle,103,216,3	script	Goruro#ep19in	4_EP19_IWIN,{
	mes "It's plucking it's feather without saying a word.";
	close;
}

icecastle,116,147,5	script	Sunyari#e19c00	21514,{
	npctalk "Did you see the fool fishing by the waterfall over there? The fishes are all here.","",bc_self;
	end;
}

icecastle,130,136,5	script	Sororyo#ep19in	21514,{
	mes "[Sororyo]";
	mes "Don't come any closer, human.";
	mes "I'm practicing a spear technique that won't slip even through ice.";
	mes "You might get hurt.";
	close;
}

icecastle,129,121,3	script	Horaryong#1	4_EP19_IWIN,{
	mes "The Iwin child is looking into the sea with sparkling eyes.";
	mes "I think it's watching a fish.";
	close;
}

icecastle,131,121,3	script	Doryarong#1	4_EP19_IWIN,{
	mes "The Iwin child is looking into the sea with sparkling eyes.";
	mes "I think it's watching a fish.";
	close;
}

icecastle,62,229,6	script	Orarang#icecastle	21513,{
	npctalk "Orarang : Huh?! An outsider!!","",bc_self;
	mes "[Feather Lodge Orarang]";
	mes "Welcome. This is a feather lodge.";
	mes "As an outsider, you pay Zeny to get services, yes? For a rest it's 5000z!!";
	next;
	switch(select("Save.:Take a rest - 5000z:Quit.")){
		case 1:
			emotion ET_HNG,getnpcid(0);
			mes "[Feather Lodge Orarang]";
			mes "The Ice Castle is my home. You can stay here and relax.";
			savepoint "icas_in",87,115;
			close;
			
		case 2:
			if(Zeny < 5000){
				emotion ET_FRET,getnpcid(0);
				mes "[Feather Lodge Orarang]";
				mes "You can't do this without money.";
				close;
			}
			mes "[Feather Lodge Orarang]";
			mes "It's warm and cozy~";
			close2;
			Zeny -= 5000;
			warp "icas_in",87,114;
			end;
			
		case 3:
			emotion ET_HNG,getnpcid(0);
			mes "[Feather Lodge Orarang]";
			mes "Hmp~ If you don't like it then don't!";
			close;
	}
}

//= Ice Inn
icas_in,95,57,6	script	Eagle Patrol#iwp1	4_BULLETIN_BOARD2,{
	npctalk "[Eagle Patrol]","",bc_self;
	end;
}

icas_in,95,62,6	duplicate(Eagle Patrol#iwp1)	Eagle Patrol#iwp2	4_BULLETIN_BOARD2

icas_in,119,62,3	script	Vellgunde Laboratory	4_BULLETIN_BOARD2,{
	npctalk "Vellgunde Laboratory","",bc_self;
	end;
}

icas_in,175,63,3	script	Hoyoyo#ep19trader	4_EP19_IWIN,{
	if(isbegin_quest(17649) < 2){
		cutin "ep19_iwin02.png",2;
		mes "[Hoyoyo]";
		mes "You're the adventurer who came the Ice Castle recently.";
		mes "My original name is Hoyoyo Hoyoyo Hoyoyo(Royal Signature Bergamot).";
		next;
		select("Hoyoyo? Hoyo?");
		mes "[Hoyoyo]";
		mes "Yes. Even if I come up with a cool name, it would sound Hoyo to you humans.";
		mes "Please just call me Hoyoyo.";
		close3;
	}
	cutin "ep19_iwin02.png",2;
	mes "[Hoyoyo]";
	mes "I'm researching Jormungandr's Mana with Vellgunde.";
	mes "There are cases that Vellgunde will need a raw manastone, I'll take care it for you if you need it.";
	next;
	switch(select("Exchange Snow Flower Ores:Exchange Snow Flower Petals:Is Hoyoyo really your name?")){
		case 1:
			mes "[Hoyoyo]";
			mes "I can exchange your Snow Flower Mana Ores for Snow Flower Mana Stones and your Snow Flower Mana Stones for a Shining Snow Flower Mana Stone.";
			mes "What can you use it for? Vellgunde can make something good out of it, right?";
			close2;
			cutin "",255;
			callshop "Ep19_Mana_Stones",1;
			end;
			
		case 2:
			mes "[Hoyoyo]";
			mes "What? Do you want to use your Snow Flower Petals?";
			mes "Vellgunde has been working on a lot of things lately.";
			if(getreputation("Ice Castle") < 1000){
				next;
				mes "[Hoyoyo]";
				mes "But wouldn't it be better if you put more effort into the Ice Castle?";
				mes "Hoyoyoyoyoyo~ Hoyoyoyoyoyo~";
				close3;
			}
			close2;
			cutin "",255;
			if(getreputation("Ice Castle") >= 2000)
				callshop "Ep19_Rep_2",1;
			else
				callshop "Ep19_Rep_1",1;
			end;
			
		case 3:
			mes "My original name is Hoyoyo Hoyoyo Hoyoyo(Royal Signature Bergamot).";
			next;
			select("Hoyoyo? Hoyo?");
			mes "[Hoyoyo]";
			mes "Yes. Even if I come up with a cool name, it would sound Hoyo to you humans.";
			mes "Please just call me Hoyoyo.";
			close3;
	}
	end;
}

-	shop	Ep19_Tool	-1,611:-1,602:-1,601:-1,23280:-1,23288:-1,645:-1,656:-1,657:-1,501:-1,502:-1,503:-1,504:-1,610:-1,713:-1,717:-1,1771:-1
-	shop	Ep19_WeaponA	-1,no,600027:150000,630018:150000,500049:150000,500050:150000,530025:150000,620017:150000,590038:150000,590039:150000,510061:150000,510062:150000,700052:150000,560032:150000,540049:150000,610037:150000,570029:150000,580030:150000
-	shop	Ep19_WeaponB	-1,no,550069:150000,640033:150000,550070:150000,650025:150000,800014:150000,810010:150000,820008:150000,830013:150000,840009:150000

icas_in,180,61,5	script	Maram#ep19trader	4_EP18_MARAM,{
	cutin "ep18_maram_01.png",2;
	if(isbegin_quest(18126) < 2){
		mes "[Maram]";
		mes "I have one job here.";
		mes "Bring the supplies that you're going to use by air.";
		mes "Wouldn't it be convenient to have things that's from the continent?";
		close3;
	}
	mes "[Maram]";
	mes "Do you need anything?";
	mes "I have airlifted items that adventurers commonly use.";
	next;
	switch(select("Tool and miscellaneous goods.:Look at the equipments.:Look at the weapons.:Dispose belongings.:What's happening?")){
		case 1:
			mes "[Maram]";
			mes "I've prepared enough, so look as much as you need.";
			close2;
			cutin "",255;
			callshop "Ep19_Tool",1;
			end;
			
		case 2:
			mes "[Maram]";
			mes "These are the armors made by Vellgunde.";
			mes "It was made with using Snow Flower's mana, that's why I'm exchanging it for Snow Flower Petals.";
			close2;
			cutin "",255;
			callshop "Ep19_Armor",1;
			end;
			
		case 3:
			mes "[Maram]";
			mes "These are the weapons that I airlifted with the other basic materials and completed by Vellgunde.";
			mes "I guess I'll have to get a small amount of Zeny for the basic materials.";
			next;
			if(select("Look at melee weapons, bows, and instruments.:Look at shurikens, guns, and staffs.") == 1)
				.@shop$ = "Ep19_WeaponA";
			else
				.@shop$ = "Ep19_WeaponB";
			close2;
			cutin "",255;
			callshop .@shop$,1;
			end;
			
		case 4:
			mes "[Maram]";
			mes "If you have unnecessary stuffs, I will buy it for you.";
			close2;
			callshop "Ep19_Tool",2;
			end;
			
		case 5:
			mes "[Maram]";
			mes "I've been thinking about what I can do here....";
			mes "Apparently, I only have been involved in supply procurement.";
			next;
			mes "[Maram]";
			mes "Since there was a shortage of existing supplies here, I decided to regularly and deliver it using the airship with Gingers help.";
			next;
			cutin "ep18_maram_02.png",2;
			mes "[Maram]";
			mes "I also helped Vellgunde open a store.";
			mes "Since there is a demand, I'm accepting Zeny as payment.";
			next;
			cutin "ep18_maram_01.png",2;
			mes "[Maram]";
			mes "Don't worry and I'll always fill it up, so do what you want with no worries!";
			close3;
	}
	end;
}

icas_in,185,63,3	script	Vellgunde#ep19vell02	4_EP19_VELLGUNDE,{
	cutin "ep19_vellgunde01.png",2;
	if(isbegin_quest(18126) < 2){
		mes "[Vellgunde]";
		mes "Are you the adventurer who came to the continent this time?";
		mes "Me? I'm Vellgunde Gaebolg. This is where we study the mana of Jormungandr.";
		close3;
	}
	if(isbegin_quest(18130) == 1){
		if(countitem(1000706) >= 3){
			mes "[Vellgunde]";
			mes "What's going on?";
			mes "Oh, you brought a Purified Mana Core!";
			mes "Thank you every time.";
			mes "It's definitely comfortable with more people.";
			next;
			cutin "ep19_vellgunde03.png",2;
			mes "[Vellgunde]";
			mes "I'll give you this Snowflower Petals instead.";
			mes "You'll need it to strengthen your weapons and equipments.";
			delitem 1000706,3;
			erasequest 18130;
			setquest 18131;
			addreputation("Ice Castle",5);
			getitem 1000608,2 + (getreputation("Ice Castle") >= 3000 ? 1 : 0);
			next;
		}
	}
	mes "[Vellgunde]";
	mes "Do you have any business with me?";
	mes "I'm studying the mana of Jormungandr and the Rgans.";
	mes "If there's anything we can do, let's help each other.";
	next;
	switch(select("Procure research materials for Vellgunde.:Snow Flower Mana Stone Extraction:Extract Snow Flower Stone Mana:Enchant Snow Flower and Glacier Equipment")){
		case 1:
			switch(checkquest(18131,PLAYTIME)){
				case -1:
					break;
					
				case 0:
				case 1:
					cutin "ep19_vellgunde02.png",2;
					mes "[Vellgunde]";
					mes "Huh?, No, it's fine.";
					mes "I still have enough mana core from the one you brought me.";
					mes "I'll request it If I need it again later.";
					close3;
					
				case 2:
					erasequest 18131;
					break;
			}
			if(isbegin_quest(18130) == 0){
				cutin "ep19_vellgunde01.png",2;
				mes "[Vellgunde]";
				mes "All I need is a Purified Mana Core of a Rgan.";
				mes "If you get ride a Rgan, you'll get a Low-Grade Mana Core.";
				next;
				mes "[Vellgunde]";
				mes "It's a crystallize form of Jormungandr's remaining mana, I think?";
				mes "If you purify it well, you can use it like a Snow Flower Petal.";
				next;
				cutin "ep19_vellgunde02.png",2;
				mes "[Vellgunde]";
				mes "Of course, it's less efficient and can be used as firewood even.";
				mes "I need it because I'm researching the mana composition of the Jormungandr and the Rgans.";
				next;
				mes "[Vellgunde]";
				mes "Collect it from a Low-Grade Mana Core and bring it to ^4D4DFFFriederike^, the one from the cult.";
				mes "She knows how to purify it, can you ask her to purify it and bring it to me?";
				mes "^4D4DFF3^000000 should be enough.";
				next;
				if(select("Okay.:Uhm.") == 2){
					cutin "ep19_vellgunde01.png",2;
					mes "[Vellgunde]";
					mes "It okay. You don't need to feel pressured to do it.";
					mes "I can order Lehar to do it.";
					close3;
				}
				cutin "ep19_vellgunde01.png",2;
				mes "[Vellgunde]";
				mes "I look forward to your cooperation.";
				setquest 18130;
				close3;
			}
			if(isbegin_quest(18130) == 1){
				if(countitem(1000706) < 3){
					mes "[Vellgunde]";
					mes "Collect it from a Low-Grade Mana Core and bring it to ^4D4DFFFriederike^000000, the one from the cult.";
					mes "She knows how to purify it, can you ask her to purify it and bring it to me?";
					mes "^4D4DFF3^000000 should be enough.";
					next;
					cutin "ep19_vellgunde02.png",2;
					mes "[Vellgunde]";
					mes "Rgans Low-Grade Mana Core can be obtained by eliminating Primitive or Low-Grade Rgans.";
					close3;
				}
			}
			end;

		case 2:
			mes "[Vellgunde]";
			mes "Do you need an Ice Flower Mana Stone?";
			mes "It can be made using Snowflower Petals...";
			mes "But you need a little luck.";
			next;
			mes "[Vellgunde]";
			mes "If you can afford a little bad luck, I can make it for you.";
			mes "I need to grind about ^4D4DFF35 Snowflower Petal^000000 and compress it.";
			next;
			if(select("Extract.:I don't need it.") == 2){
				cutin "ep19_vellgunde02.png",2;
				mes "[Vellgunde]";
				mes "Come to me later if you need anything.";
				close3;
			}
			if(countitem(1000608) < 35){
				mes "[Vellgunde]";
				mes "You need 35 Snowflower Petal to make an Snow Flower Mana Stone.";
				mes "Isn't that the beauty of mana?";
				mes "That's why it's used here as a fuel and other things.";
				next;
				mes "[Vellgunde]";
				mes "It's also acts as currency among the Iwins, so do some work for the Iwins around you.";
				mes "You might get your hand on some Flower Petals.";
				close3;
			}
			.@item_id = rand(1000711,1000810);
			getitem .@item_id,1;
			mes "[Vellgunde]";
			mes "Now, here's the result of the Mana Stone.";
			mes "Did it come out as you've expected?";
			delitem 1000608,35;
			specialeffect2 EF_SPHERE;
			next;
			while(true){
				if(countitem(1000608) < 35){
					mes "[Vellgunde]";
					mes "You need 35 Snowflower Petal to make an Snow Flower Mana Stone.";
					mes "Isn't that the beauty of mana?";
					mes "That's why it's used here as a fuel and other things.";
					next;
					mes "[Vellgunde]";
					mes "It's also acts as currency among the Iwins, so do some work for the Iwins around you.";
					mes "You might get your hand on some Flower Petals.";
					close3;
				}
				mes "[Vellgunde]";
				mes "Are you going to extract some more Mana Stone?";
				next;
				if(select("Extract.:I don't need it.") == 2){
					cutin "ep19_vellgunde02.png",2;
					mes "[Vellgunde]";
					mes "Come to me later if you need anything.";
					close3;
				}
				.@item_id = rand(1000711,1000810);
				getitem .@item_id,1;
				mes "[Vellgunde]";
				mes "Now, here's the result of the Mana Stone.";
				mes "Did it come out as you've expected?";
				delitem 1000608,35;
				specialeffect2 EF_SPHERE;
				next;
			}
			end;

		case 3:
			mes "[Vellgunde]";
			mes "You want to extract the Snow Flower Stone's Mana...?";
			mes "Did you bring all the materials?";
			mes "First of all, try putting something in the extractor here.";
			close2;
			//open_laphine_synthesis(101184);
			end;
		
		case 4:
			mes "[Vellgunde]";
			mes "I mean. When I started research the mana of Rgans and Jormungandr, I didn't know that they'd even touch a weapon like this.";
			mes "Anyway, you're a big help, so I should help you in anyway I can, right?";
			next;
			mes "[Vellgunde]";
			mes "What equipment do you want to empower?";
			next;
			.@id = 25;
			.@s = select("Snowflower Armor/Robe:Snowflower Manteau/Muffler:Snowflower Boots/Shoes:Snowflower Pendant/Necklace:Snowflower Ring/Earring:Glacier Weapon");
			.@id += .@s;
			switch(.@s){
				case 1:
				case 2:
					mes "[Vellgunde]";
					mes "Good, good.";
					mes "It's an equipment that I put all my effort into.";
					mes "Let's do something about it.";
					break;
					
				case 3:
					mes "[Vellgunde]";
					mes "Ah, shoes?";
					mes "Shoes are definitely important in a place like this.";
					mes "Come, what do you want to do with it?";
					break;
					
				case 4:
					mes "[Vellgunde]";
					mes "These accessories are the best thing to be imbued with mana.";
					mes "These jewelries are compatible with mana.";
					break;
					
				case 5:
				case 6:
					mes "[Vellgunde]";
					mes "Anything will stick to a Glacier weapons.";
					mes "I mean it may be anything.";
					break;
			}
			close2;
			cutin "",255;
			//enchantui_open(.@id);
			end;	
	}
	end;
	
	
OnInit:
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(18126) == 2 && isbegin_quest(18130) == 0 && checkquest(18131,PLAYTIME) == -1";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(18126) == 2 && isbegin_quest(18130) == 0 && checkquest(18131,PLAYTIME) == 2";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(18130) == 1 && countitem(1000706) >= 3";
end;
}

//= New Merchant???
/*
icas_in,188,60,3	script	Poruru#ep19	4_EP19_IWIN,{
	if(isbegin_quest(17649) < 2){
		cutin "ep19_iwin11.png",2;
		mes "[Poruru]";
		mes "I'm researching something interesting that I've recently discovered.";
		mes "I'm Vellgunde's assistant ever since....";
		mes "I think there will be good results soon.";
		close3;
	}
}
*/

icas_in,51,68,4	script	Private Dorarong#s_iw	21513,{
	cutin "ep19_iwin05.png",2;
	npctalk "...z ....Z ...Z ...z","",bc_self;
	sleep2 1000;
	unittalk getcharid(3),"It's sound asleep. Is it tired?",bc_self;
	cutin "",255;
	end;
}

icas_in,51,53,8	script	Trainee Hororung#s_iw	21513,{
	cutin "ep19_iwin05.png",2;
	npctalk "z Z z Z z","",bc_self;
	sleep2 1000;
	unittalk getcharid(3),"It's...",bc_self;
	cutin "",255;
	end;
}

icas_in,42,68,4	script	Private Torari#s_iw	21513,{
	cutin "ep19_iwin05.png",2;
	npctalk "zz zZz zz zZz";
	sleep2 1000;
	unittalk getcharid(3),"It's sound asleep. Is it tired?",bc_self;
	cutin "",255;
	end;
}

icas_in,42,53,8	script	Trainee Pororong#s_iw	21513,{
	cutin "ep19_iwin05.png",2;
	npctalk "Z.....z....z...","",bc_self;
	sleep2 1000;
	unittalk getcharid(3),"Is it asleep?",bc_self;
	cutin "",255;
	end;
}

icas_in,33,68,4	script	???? Gorori#s_iw	21513,{
	cutin "ep19_iwin05.png",2;
	npctalk "zzzz..ZZ..zzz","",bc_self;
	sleep2 1000;
	unittalk getcharid(3),"I think it's sleeping.",bc_self;
	cutin "",255;
	end;
}

icas_in,33,53,8	script	Private Horiryu#in_iwp	21513,{
	if(isbegin_quest(5972) < 2){
		cutin "ep19_iwin05.png",2;
		npctalk "z Z zz Z..","",bc_self;
		mes "The Iwin is sitting on the bed and dozing off.";
		close3;
	}
	if(isbegin_quest(5980) == 0){
		mes "In the patrol barracks where most of the Iwin sleeps, one Iwin is moving alone busily. What's going on?";
		next;
		select("Try to talk to him.");
		cutin "ep19_iwin08.png",2;
		emotion ET_BLABLA ,getnpcid(0,"Private Horiryu#in_iwp");
		npctalk "Private Horiryu: Private Horiryu!","Private Horiryu#in_iwp",bc_self;
		sleep2 1000;
		cutin "ep19_iwin09.png",2;
		mes "[Private Horiryu]";
		mes "Woah! What, you're not a senior Iwin. Ah.. so sleepy..";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "What's going on?";
		next;
		cutin "ep19_iwin07.png",2;
		emotion ET_FRET,getnpcid(0,"Private Horiryu#in_iwp");
		mes "[Private Horiryu]";
		mes "Have we met before? Why are you suddenly talking to me informally?";
		next;
		emotion ET_THINK,getnpcid(0,"Private Horiryu#in_iwp");
		emotion ET_THINK,getcharid(3);
		sleep2 3000;
		cutin "ep19_iwin10.png",2;
		mes "[Private Horiryu]";
		mes "Hahaha, don't be scared. It's a joke to see if there's an issue.";
		next;
		mes "[Private Horiryu]";
		mes "Let me explain the situation. As you can see my adventurer friend, everything is ice around here.";
		next;
		cutin "ep19_iwin05.png",2;
		emotion ET_OTL,getnpcid(0,"Private Horiryu#in_iwp");
		mes "[Private Horiryu]";
		mes "Everywhere I look, there's only ice! It's cold! Even for something cold, it's too cold!!";
		next;
		cutin "ep19_iwin10.png",2;
		mes "[Private Horiryu]";
		mes "Then I heard rumors about adventurers coming from the outside..";
		next;
		cutin "ep19_iwin04.png",2;
		mes "[Private Horiryu]";
		mes "Before that my adventurer friend, I'd like to ask you a question.. the outside.. is it a world full of warm feathers?";
		next;
		emotion ET_QUESTION,getcharid(3);
		mes "[" + strcharinfo(0) + "]";
		mes "Yes, it's warm.. what?! Feather?";
		next;
		emotion ET_QUESTION,getnpcid(0,"Private Horiryu#in_iwp");
		cutin "ep19_iwin09.png",2;
		mes "[Private Horiryu]";
		mes "Why are you surprised? Of course it is.";
		mes "Isn't the formula [ Warm = Feathers ]?";
		next;
		mes "[Private Horiryu]";
		mes "Anyway! My adventurer friends seems to be free to go outside, so please get some feathers from the outside.";
		next;
		cutin "ep19_iwin11.png",2;
		mes "[Private Horiryu]";
		mes "If you bring 10 of the same kind, I'll get you a appropriate reward for the feathers condition.";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "Why 10?";
		next;
		mes "[Private Horiryu]";
		mes "It's a secret change of supplies. Yes. I'll change mine first and the recruits.";
		next;
		mes "[Private Horiryu]";
		mes "What do you say? Can you help me?";
		next;
		if(select("Piece of cake.:I'm a bit busy..") == 2){
			setquest 5980;
			completequest 5980;
			emotion ET_HNG,getnpcid(0,"Private Horiryu#in_iwp");
			mes "[Private Horiryu]";
			mes "There's nothing I can do if you're busy. Please make some time for us later or we'll stay cold.";
		} else {
			setquest 5980;
			completequest 5980;
			setquest 5982;
			mes "[Private Horiryu]";
			mes "Let's do it secretly.. Okay?";
		}
		close3;
	}
	switch(checkquest(5981,PLAYTIME)){
		case -1:
			break;
			
		case 0:
		case 1:
			mes "[Private Horiryu]";
			mes "Keep it a secret, alright? Let's talk about this again tomorrow.";
			close3;
			
		case 2:
			npctalk "Private Horiryu: Shh! It has to be a secret, okay?","Private Horiryu#in_iwp",bc_self;
			emotion ET_SPARK,getnpcid(0);
			erasequest 5981;
			break;
	}
	if(isbegin_quest(5982) == 0){
		cutin "ep19_iwin10.png",2;
		mes "[Private Horiryu]";
		mes "Could you secretly bring me feathers?";
		next;
		if(select("I'll bring it.:I'm busy.") == 2){
			mes "[Private Horiryu]";
			mes "There's nothing I can do if you're busy. Plesae make some time for us later or we'll stay cold.";
			close3;
		}
		cutin "ep19_iwin09.png",2;
		setquest 5982;
		mes "[Private Horiryu]";
		mes "Let's do it secretly.. Okay?";
		close3;
	}
	if(isbegin_quest(5982) == 1){
		cutin "ep19_iwin10.png",2;
		mes "[Private Horiryu]";
		mes "Adventurer friend, did you bring the feathers?";
		next;
		if(select("I brought it.:Not yet..") == 2){
			mes "[Private Horiryu]";
			mes "Good! Any 10 feather of the same kind will do. I'll wait for you!";
			close3;
		}
		mes "[Private Horiryu]";
		mes "Let me take a look at your bag for a moment.";
		next;
		setarray .@feather,916,6405,7063,7101,7115,6393,6691,7440,7441;
		setarray .@class,1,2,2,2,2,3,3,3,3;
		while(true){
			for(.@i = 0; .@i < getarraysize(.@feather); .@i++){
				if(countitem(.@feather[.@i]) >= 10){
					switch(.@class[.@i]){
						case 1:
							mes "[Private Horiryu]";
							mes "What? This crude feather is not much different from the feather in the supply, it's warmness is in the lower grade.";
							break;
							
						case 2:
							mes "[Private Horiryu]";
							mes "What? This decent feather is a little bit better from the feather in the supply, it's warmness is in the intermediate grade.";
							break;
							
						case 3:
							mes "[Private Horiryu]";
							mes "Woah? This luxurious feather is very high quality compare to those in the supply, it's warmness is top-notch.";
							break;
					}
					mes "[Private Horiryu]";
					mes "Will you give me these <ITEM>[" + getitemname(.@feather[.@i]) + "]<INFO>" + .@feather[.@i] + "</INFO></ITEM>?";
					next;
					if(select("Yes.:I brought another feather.") == 1){
						delitem .@feather[.@i],10;
						erasequest 5982;
						setquest 5981;
						addreputation("Ice Castle",5);
						getitem 1000608,2 + (getreputation("Ice Castle") >= 3000 ? 1 : 0);
						switch(.@class[.@i]){
							case 1: getexp 39182846,3600000; break;
							case 2: getexp 45713321,4500000; break;
							case 3: getexp 52243795,5400000; break;
						}
						mes "[Private Horiryu]";
						mes "This is the amount you'll receive for <ITEM>[" + getitemname(.@feather[.@i]) + "]<INFO>" + .@feather[.@i] + "</INFO></ITEM>. The compensation is without negotation, so no complaining.";
						next;
						cutin "ep19_iwin10.png",2;
						mes "[Private Horiryu]";
						mes "Good! I'll swap the <ITEM>[" + getitemname(.@feather[.@i]) + "]<INFO>" + .@feather[.@i] + "</INFO></ITEM> that you brought me today to the supplies, please bring me feathers again tomorrow.";
						close3;
					}
					mes "[Private Horiryu]";
					mes "Another feather..";
					next;
					continue;
				}
			}
			cutin "ep19_iwin10.png",2;
			mes "[Private Horiryu]";
			mes "Looks like there's no other feathers in your bag. Shall we take a look again?";
			next;
			if(select("Take a look again.:Quit.") == 2){
				mes "[Private Horiryu]";
				mes "Good! Any 10 feather of the same kind will do. I'll wait for you!";
				close3;
			}
		}
	}
	end;
	
OnInit:
	setarray .@feather,916,6405,7063,7101,7115,6393,6691,7440,7441;
	for(.@i = 0; .@i < getarraysize(.@feather); .@i++)
		.@var$ += "countitem(" + .@feather[.@i] + ") >= 10 " + (.@i == getarraysize(.@feather) - 1 ? "":"||");
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(5972) == 2 && isbegin_quest(5980) == 0";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(5980) == 2 && isbegin_quest(5982) == 0 && checkquest(5981,PLAYTIME) == -1";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(5980) == 2 && isbegin_quest(5982) == 0 && checkquest(5981,PLAYTIME) == 2";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(5982) == 1 && " + .@var$;
end;
}

icas_in,142,201,3	script	Toruru#ep19iwin05	21513,{
	cutin "ep19_iwin03.png",2;
	mes "[Toruru]";
	mes "In the past. Did Varmundt really oftenly come and see us?";
	mes "There's a human only accomodation and restaurant that was made that time.";
	next;
	mes "[Toruru]";
	mes "The restaurant is on the left, and the accommodation is on the right.";
	mes "Got it?";
	close3;
}
icas_in,109,242,5	script	Gyururu#ep19iwin05	21513,{
	cutin "ep19_iwin04.png",2;
	mes "[Gyururu]";
	mes "It's nice to see a lot of people come these days.";
	mes "I didn't even notice that time was passing before you people came.";
	mes "Leon and Aurelie were sleeping...";
	next;
	select("Did they slept?");
	mes "[Gyururu]";
	mes "They wake up if something happens like a ghost.";
	mes "They can see everything in their dreams.";
	mes "Isn't it amazing?";
	close3;;
}

icas_in,48,252,3	script	Miriam#ep19miriam05	4_EP18_MIRIAM,{
	if(isbegin_quest(18133) == 0){
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "I think Leon has something to say.";
		close3;
	}
	if(isbegin_quest(18133) == 1){
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "I think we can search the Frozen Scale Plain and the Glacier area. What do you think?";
		next;
		select("What will we do with the victims?");
		mes "[Miriam]";
		mes "Bring them to the Ice Castle's accommodation first to cool off, and we'll send them back to the continent through Lazy and Maram.";
		next;
		mes "[Miriam]";
		mes "The adventurer will take one area, and I'll take the other.";
		mes "Let's ask the Iwin and Lehar to search the Rgans Nest.";
		next;
		mes "[Miriam]";
		mes "I think they should search places where they might possibly be.";
		mes "It will shorten our search time.";
		next;
		mes "[Miriam]";
		mes "The adventurer and I will search the places with difficult environment for the victims.";
		mes "Talk to me when you're ready.";
		completequest 18133;
		questinfo_refresh;
		close3;
	}
	switch(checkquest(18137,PLAYTIME)){
		case -1:
			break;
			
		case 0:
		case 1:
			cutin "ep18_miriam_01.png",0;
			mes "[Miriam]";		
			mes "You can't wander around the snowy field forever.";
			mes "You better get plenty of rest before we start searching again.";
			close3;
			
		case 2:
			erasequest 18137;
			break;
	}
	if(isbegin_quest(18136) == 1){
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "The rescued survivors had been escorted to the adventurer's quarters.";
		mes "Maram will be waiting for you there.";
		close3;
	}
	if(isbegin_quest(18134) > 0 || isbegin_quest(18135) > 0){
		if(isbegin_quest(18136) == 0){
			cutin "ep18_miriam_01.png",0;
			mes "[Miriam]";
			if(isbegin_quest(18134)){
				mes "Please search Frozen Glacier area.";
				mes "I'll search in the Plains.";	
			} else {
				mes "Please search Frozen Plains area.";
				mes "I'll search in the Glaciers.";	
			}
			mes "If you find any survivor, bring them back to the Ice Castle accommodation immediately.";
			close3;
		}
	}
	cutin "ep18_miriam_01.png",0;
	mes "[Miriam]";
	mes "Shall we divide the search area for survivors?";
	mes "You can choose between the Glaciers or Plains, I'll search the other.";
	next;
	if(select("Search the Glacier.:Search the Plain.") == 1){
		mes "[Miriam]";
		mes "Then, I'll search the Plain area.";
		.@qid = 18135;
	} else {
		mes "Then, I'll search the Glacier area.";
		.@qid = 18134;
	}
	mes "If you find a survivor, bring him back to the Ice Castle's accommodation immediately.";
	next;
	mes "[Miriam]";
	mes "I already told Maram beforehand.";
	mes "If we find one, we will make sure that they return immediately.";
	setquest .@qid;
	close3;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(18133) == 1";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(18133) == 2 && isbegin_quest(18134) == 0 && isbegin_quest(18135) == 0 && isbegin_quest(18136) == 0 && checkquest(18137,PLAYTIME) == -1";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(18133) == 2 && isbegin_quest(18134) == 0 && isbegin_quest(18135) == 0 && isbegin_quest(18136) == 0 && checkquest(18137,PLAYTIME) == 2";
end;
}

icas_in,42,252,5	script	Leon Petit#ep19leon02	4_EP19_LEON,{
	if(isbegin_quest(18132) == 0){
		mes "[Leon]";
		mes "Do you need anything?";
		mes "The others will be in the accomodation or restaurant.";
		close;
	}
	if(isbegin_quest(18132) == 1){
		cutin "ep19_leon01.png",2;
		mes "[Leon]";
		mes "Did you get the message from Lehar?";
		mes "I apologize for making you come here in the middle of your busy schedule.";
		next;
		cutin "ep19_leon04.png",2;
		mes "[Leon]";
		mes "I have a separate request for you.";
		mes "Do you remember the laborers that we rescued when we searched the Rgans nest last time?";
		next;
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "Those who were rescued that time were led back by Maram and Lazy, but..?";
		next;
		cutin "ep19_leon01.png",2;
		mes "[Leon]";
		mes "It seems that there are more who escaped from the Rgans nest during that time.";
		mes "The Iwin Patrol seems to have found some of them in the Glacier and the Plain.";
		next;
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "It's definitely possible.";
		mes "There might be more survivors.";
		next;
		cutin "ep19_leon01.png",2;
		mes "[Leon]";
		mes "Although the Iwins are patrolling, it's hard to look at every area, so I'd like to ask for your help.";
		next;
		cutin "ep19_leon05.png",2;
		mes "[Leon]";
		mes ".....";
		next;
		select("Is there anything bothering you?");
		mes "[Leon]";
		mes "We it so it will completely separate Midgard from Isgard.";
		mes "To be clear, it was made to separate the Rgans....";
		next;
		cutin "ep19_leon01.png",2;
		mes "[Leon]";
		mes "The fact there are people who are able to come here without an invitation is a clear proof that the barrier has weakened.";
		mes "I lived for hundreds of years without dying, it just crossed my mind that my existence was for this day.";
		next;
		select("Hundreds of years..");
		cutin "ep19_leon01.png",2;
		mes "[Leon]";
		mes "Aurelie and I are guarding this place, following the will of Jormungandr.";
		mes "As you know, Jormungandr is not an evil god.";
		next;
		cutin "ep19_leon04.png",2;
		mes "[Leon]";
		mes "The reason Jormungandr was considered evil was because of the Rgans who called themselves the Jormungandr cult, plunge the continent into chaos.";
		next;
		select("What's the curse of Jormungandr?");
		cutin "ep19_leon05.png",2;
		mes "[Leon]";
		mes "The head of the Jormungandr cult casted a blood curse as his last act.";
		mes "They always had Jormungandr on their banners, that's how it was passed down.";
		next;
		cutin "ep19_leon01.png",2;
		mes "[Leon]";
		mes "Anyways, Jormungandr is god who helped save the continent by sacrificing his own body.";
		mes "In order for us to fullfill Jormungandr's will, the Rgans must not leave this land.";
		next;
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "It's a heavy story.";
		mes "I understand that the recorded history is not true, but...";
		next;
		cutin "ep19_leon01.png",2;
		mes "[Leon]";
		mes "Then and now, we just have to do the best that we can, right?";
		mes "Aurelie and I will keep the balance.";
		next;
		cutin "ep19_leon01.png",2;
		mes "[Leon]";
		mes "Can you rescue the survivors first?";
		mes "I'll leave them to you.";
		next;
		cutin "ep19_leon03.png",2;
		mes "[Leon]";
		mes "Of course, we plan to do everything we can to support you.";
		mes "We have a continent to protect.";
		mes "Shouldn't we protect our descendants?";
		next;
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "I understand what you mean, Leon.";
		mes "Adventurer, shall we make a detailed plan?";
		completequest 18132;
		setquest 18133;
		questinfo_refresh;
		close3;
	}
	if(isbegin_quest(18133) == 1){
		cutin "ep19_leon03.png",2;
		mes "[Leon]";
		mes "It's a relief that the castle has more manpower.";
		mes "Lehar was working hard alone before.";
		mes "Then, let's talk about the details with Miriam.";
		close3;
	}
	cutin "ep19_leon01.png",2;
	mes "[Leon]";
	mes "Is it strange to have a fireplace in a snowy place where not a single grass grows?";
	mes "The crystallized mana that are sometimes found around here, the Snowflower Petal, and the mana cores from the Rgans are used as a fuel.";
	next;
	mes "[Leon]";
	mes "Come and warm yourself.";
	close3;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(18132) == 1";
end;
}

icas_in,262,179,3	script	Maram#ep19maram05	4_EP18_MARAM,{
	if(isbegin_quest(18136) == 1){
		cutin "ep18_maram_01.png",2;
		mes "[Maram]";
		mes "The rescued survivors are warming up.";
		mes "I'm going to take them into the airship.";
		next;
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "I'm glad we found them before anything big happened.";
		mes "It seems that there are people who follows the Jormungandr cult because it's difficult to make a living.";
		next;
		cutin "ep18_maram_01.png",2;
		mes "[Maram]";
		mes "I see. Anyway, Lazy and I will investigate them, and send them home.";
		mes "You did a great job. Go get some rest.";
		next;
		cutin "ep18_miriam_01.png",0;
		mes "[Miriam]";
		mes "Yes.";
		mes "Let's rest for today, and search again tomorrow.";
		erasequest 18136;
		setquest 18137;
		addreputation("Ice Castle",5);
		getitem 1000608,2 + (getreputation("Ice Castle") >= 3000 ? 1 : 0);
		close3;
	}
	cutin "ep18_maram_01.png",2;
	mes "[Maram]";
	mes "We'll wait until the survivors find some stability before we leave.";
	mes "If you need anything, I'll go and buy it for you.";
	close3;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(18136) == 1";
end;
}

icas_in,260,180,5	script	Miriam#ep19miriam06	4_EP18_MIRIAM,{
	cutin "ep18_miriam_01.png",0;
	mes "[Miriam]";
	mes "Was it a hard search?";
	mes "Why don't you warm up for a while?";
	close3;
}

icas_in,261,186,7	script	Rescued Survivor#ep19_01	4_M_SITDOWN,{ npctalk "Tha.. Thank you for saving me. I.. thought I was going to freeze to death.","",bc_self; end; }
icas_in,262,184,7	script	Rescued Survivor#ep19_02	4_F_SITDOWN,{ npctalk "..I..I missed this. The taste of warm fire..!","",bc_self; end; }

icas_in,236,45,5	script	Goryaru#ep19in	4_EP19_IWIN,{
	mes "[Goryaru]";
	mes "I want to eat more seaweed.";
	close;
}

icas_in,234,49,3	script	Hururu#ep19in	4_EP19_IWIN,{
	mes "It's busy eating.";
	close;
}

icas_in,241,49,3	script	Jororu#ep19in	4_EP19_IWIN,{
	mes "[Jororu]";
	mes "I feel sleepy after eating...";
	close;
}

icas_in,248,47,3	script	Soriri#ep19in	4_EP19_IWIN,{
	mes "[Soriri]";
	mes "Wait a minute, let's eat and play...";
	mes "No, I'm not a child. If you have the time, why don't you spend time with my kids?";
	mes "I'm sure they'll like it if it's a story from a human from the outside.";
	close;
}

icas_in,251,46,7	script	Sorarang#ep19in	4_EP19_IWIN,{
	mes "[Sorarang]";
	mes "I'll be in the patrol when I grow up!";
	mes "The patrol hat? I like it to be yellow. Red is too ugly.";
	npctalk "Koriri : Yes. That's true, red is ugly.","Koriri#ep19in",bc_self;
	close;
}

icas_in,252,50,5	script	Koriri#ep19in	4_EP19_IWIN,{
	mes "[Koriri]";
	mes "I'll be in the patrol when I grow up!";
	mes "For the patrol hat, I like green. I want to use green.";
	npctalk  "Sorarang : Yellow is cooler!","Sorarang#ep19in",bc_self;
	npctalk "Koriri : Green is cooler!","",bc_self;
	close;
}

icas_in,245,53,5	script	Jorori#ep19in	4_EP19_IWIN,{
	mes "[Jorori]";
	mes "What should I do?";
	mes "Should I pluck out my feathers?";
	close;
}

icas_in,235,57,3	script	Horyaya#ep19in	4_EP19_IWIN,{
	mes "[Horyaya]";
	mes "I'm Horyaya!";
	mes "I'll be a great guardian in the future!";
	close;
}

icas_in,232,59,5	script	Torarung#ep19in	4_EP19_IWIN,{
	mes "[Torarung]";
	mes "I'm Torarung!";
	mes "In the future...";
	mes "In the future... I'll be a great guardian!";
	mes "Is this how you say it?";
	npctalk "Horyaya : Yes!","Horyaya#ep19in",bc_self;
	npctalk "Torarung : Phew.","",bc_self;
	close;
}

icas_in,241,66,3	script	Zoryara#ep19in	4_EP19_ZORYARA,{
	if(isbegin_quest(18126) < 2){
		mes "[Zoryara]";
		mes "I've heard that humans were coming...";
		mes "Did you get the permission to go out? So to speak, we are in a state of war, it's dangerous to act alone without permission.";
		mes "Go back to the nest.";
		close;
	}
	if(isbegin_quest(8787) == 0){
		mes "[Zoryara]";
		mes "Are you the human who came with Herlock? Nice to meet you.";
		mes "My name is...";
		mes "Alvin Alexandria Adele David Cassia Harry Apollo.";
		mes "Call me what you're comfortable with.";
		next;
		mes "[Horyaya]";
		mes "My eldest brother's name is Zoryara!";
		mes "I'm Horyaya.";
		next;
		mes "[Zoryara]";
		mes "...Horyaya!";
		next;
		mes "[Zoryara]";
		mes "...Hmm.";
		mes "My assistant made a mistake.";
		next;
		mes "[Zoryara]";
		mes "As you can see, this place is a kitchen and restaurant.";
		mes "The kitchen is open to everyone, but I'm in charge of managing the ingredients.";
		mes "If it's not cooking... I'll be cutting, chopping, and slicing if necessary.";
		next;
		mes "[Zoryara]";
		mes "They're all frozen anyway...";
		mes "I'm glad I don't have to worry about it rotting.";
		mes "And using the fire, the results will be reduced.";
		mes "Whil I was managing, there were often times when we need a large quantity.";
		next;
		mes "[Zoryara]";
		mes "...I kept talking to myself.";
		mes "When Horyaya goes fishing, he has to see Moryara...";
		npctalk "Horyaya : I'm observing my brother!","Horyaya#ep19in",bc_self;
		next;
		mes "[Zoryara]";
		mes "My parents are on the patrol.";
		mes "Horyaya is mostly raised by me and our second brother Moryara.";
		npctalk "Horyaya : I was helping you!","Horyaya#ep19in",bc_self;
		next;
		mes "[Zoryara]";
		mes "Assistant. I'm talking to a guest.";
		npctalk "Horyaya : Yes!","Horyaya#ep19in",bc_self;
		next;
		mes "[Zoryara]";
		mes "Please understand.";
		npctalk "Horyaya : I want to eat Ice Straw!","Horyaya#ep19in",bc_self;
		next;
		mes "[Zoryara]";
		mes "...";
		next;
		mes "[Zoryara]";
		mes "<ITEM>[Thin Though Shell]<INFO>1000830</INFO></ITEM>... It's all out of stock.";
		mes "What if there are 20 or so...";
		npctalk "Torarung : Me too!","Torarung#ep19in",bc_self;
		next;
		mes "[Zoryara]";
		mes "...";
		next;
		mes "I almost thought this is a nursery.";
		next;
		if(select("I'll go hunt for you instead.:Comfort him.") == 2){
			mes "[Zoryara]";
			mes "It happens all the time, but...";
			mes "Yeah. It happens all the time...";
			npctalk "Torarung : Zoryara, Play with me!","Torarung#ep19in",bc_self;
			close;
		}
		mes "[Zoryara]";
		mes "Thank you so much!";
		mes "I'll take care of the kids... Ah, of course human, you have nothing to do with the children...";
		mes "You can get <ITEM>[Thin Though Shell]<INFO>1000830</INFO></ITEM> by hunting Ice Straws.";
		mes "Can you please bring me 20!"; //in KRO this is displayed as 20 but it's only asking for 15
		setquest 8787;
		close;
	}
	if(isbegin_quest(8787) == 1){
		if(countitem(1000830) < 20){
			mes "[Zoryara]"; //TODO DIALOGUE
			mes "I'll take care of the kids... Ah, of course human, you have nothing to do with the children...";
			mes "You can get <ITEM>[Thin Though Shell]<INFO>1000830</INFO></ITEM> by hunting Ice Straws.";
			mes "Can you please bring me 20!"; //in KRO this is displayed as 20 but it's only asking for 15
			close;
		}
		mes "[Zoryara]";
		mes "Thank you, human!";
		npctalk "Horyaya : Thank you, human!","Horyaya#ep19in",bc_self;
		npctalk "Thank you, human!","Torarung#ep19in",bc_self;
		delitem 1000830,20;
		completequest 8787;
		setquest 8788;
		addreputation("Ice Castle",5);
		getitem 1000608,10;
		next;
		mes "[Zoryara]";
		mes "...";
		mes "Hey...";
		emotion ET_CRY,getnpcid(0,"Zoryara#ep19in");
		next;
		mes "[Zoryara]";
		mes "Here... I'll cut it into small pieces, so stay away from me.";
		mes "It's dangerous if the ice cubes splashed.";
		mes "Human, if you're going to watch, step back a little.";
		mes "No, shall I use the knife...";
		npctalk "Zoryara : Secret move! Iwin Guardian, Quick Sword!","Zoryara#ep19in",bc_self;
		npctalk "Horyaya : Zoryara is so cool!","Horyaya#ep19in",bc_self;
		npctalk "Zoryara is so cool!","Torarung#ep19in",bc_self;
		close;
	}
	switch(checkquest(8788,PLAYTIME)){
		case -1:
			break;
			
		case 0:
		case 1:
			mes "[Zoryara]";
			mes "I'm relieved that the kids are happy.";
			mes "I can't believe another race we're an influence on our parenting method... Hehe.";
			close;
			
		case 2:
			erasequest 8788;
			break;
	}
	if(isbegin_quest(8789) == 0){
		mes "[Zoryara]";
		mes "Ho, Ho...";
		mes "Human, nice to see you again...";
		next;
		select("You're busy today too");
		mes "[Zoryara]";
		mes "No... not there.";
		npctalk "Horyaya : I want to eat Jellopy!","Horyaya#ep19in",bc_self;
		npctalk "Torarung : Me too!","Torarung#ep19in",bc_self;
		next;
		mes "[Zoryara]";
		mes "...over there.";
		mes "It's not that I can manage my inventory... They just like it too much.";
		mes "Can you get me 15 <ITEM>[Thin Though Shell]<INFO>1000830</INFO></ITEM>...";
		next;
		if(select("Accept it.:I'm busy") == 2){
			mes "[Zoryara]";
			mes "There's nothing I can do...";
			npctalk "Horyaya : Do you think the assistant will be back?","Horyaya#ep19in",bc_self;
			npctalk "Torarung : I want to go too!","Torarung#ep19in",bc_self;
			npctalk "Zoryara : You guys are still too young, come on... play over there.","Zoryara#ep19in",bc_self;
			close;
		}
		mes "[Zoryara]";
		mes "Thank you...";
		mes "You can get <ITEM>[Thin Though Shell]<INFO>1000830</INFO></ITEM> by hunting Ice Straws.";
		npctalk "Horyaya : Play with me until the human come back!","Horyaya#ep19in",bc_self;
		npctalk "Me too!","Torarung#ep19in",bc_self;
		setquest 8789;
		close;
	}
	if(isbegin_quest(8789) == 1){
		if(countitem(1000830) < 15){
			mes "[Zoryara]";
			mes "Can you get me 15 <ITEM>[Thin Though Shell]<INFO>1000830</INFO></ITEM>...";
			mes "I can't help it if the kids want to eat it.";
			npctalk "Horyaya : Zoryara can't hunt!","Horyaya#ep19in",bc_self;
			npctalk "Torarung : But he uses the knife well!","Torarung#ep19in",bc_self;
			npctalk "Zoryara : These kids...","Zoryara#ep19in",bc_self;
			close;
		}
		mes "[Zoryara]";
		mes "Thank you, human!";
		npctalk "Horyaya : Thank you, human!","Horyaya#ep19in",bc_self;
		npctalk "Thank you, human!","Torarung#ep19in",bc_self;
		delitem 1000830,15;
		erasequest 8789;
		setquest 8788;
		addreputation("Ice Castle",5);
		getitem 1000608,2 + (getreputation("Ice Castle") >= 3000 ? 1 : 0);
		next;
		mes "[Zoryara]";
		mes "...";
		mes "Hey...";
		emotion ET_CRY,getnpcid(0,"Zoryara#ep19in");
		next;
		mes "[Zoryara]";
		mes "Here... I'll cut it into small pieces, so stay away from me.";
		mes "It's dangerous if the ice cubes splashed.";
		mes "Human, if you're going to watch, step back a little.";
		mes "No, shall I use the knife...";
		npctalk "Zoryara : Secret move! Iwin Guardian, Quick Sword!","Zoryara#ep19in",bc_self;
		npctalk "Horyaya : Zoryara is so cool!","Horyaya#ep19in",bc_self;
		npctalk "Zoryara is so cool!","Torarung#ep19in",bc_self;
		close;
	}
	
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(18126) == 2 && isbegin_quest(8787) == 0";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(8787) == 1 && countitem(1000830) >= 15";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(8789) == 1 && countitem(1000830) >= 20";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(8787) == 2 && isbegin_quest(8789) == 0 && checkquest(8788,PLAYTIME) == -1";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(8787) == 2 && isbegin_quest(8789) == 0 && checkquest(8788,PLAYTIME) == 2";
end;
}

icas_in,244,63,1	script	Zororo#ep19in	4_EP19_IWIN,{
	mes "[Zororo]";
	mes "When I grow up~";
	mes "I'll help you!";
	npctalk "Zoryara : You can already help me now...","Zoryara#ep19in",bc_self;
	close;
}

icas_in,251,59,7	script	Torira#ep19in	4_EP19_IWIN,{
	mes "[Torira]";
	mes "I'll be a great adult like my dad.";
	close;
}

icas_in,81,121,4	script	Chaango#warehouse	HIDDEN_NPC,{
	unittalk getcharid(3),"Chaango?",bc_self;
	mes "It's a frozen jar with a strange name. There is a pile of change and note from the lodge owner.";
	mes "The use of storage is ^389DD4 500 Zeny^000000. Just leave it next to the jar and I'll take it later~";
	mes "Feather Lodge Orarang.";
	next;
	if(select("Storage 1:Storage 2") == 1)
		.@id = 1;
	else
		.@id = 2;
	if(Zeny < 500){
		unittalk getcharid(3),"I can't use it because I don't have any change.",bc_self;
		end;
	}
	Zeny -= 500;
	openstorage2 .@id,STOR_MODE_GET|STOR_MODE_PUT;
	end;
}

//Jor Nest

jor_nest,34,79,5	script	Mimirgand#ep19r	4_EP19_RGAN_SR1,{
	if(!191_scroll_check()){
		mes "[Rgan Bishop]"; //TODO GET ORIGINAL DIALOGUE
		mes "Humans shouldn't come here. Get out of this place.";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	if(isbegin_quest(17624) < 2){
		mes "[Mimirgand]";
		mes "Hmm.";
		mes "Don't the non-believers know?";
		mes "The grace of Jormungandr...";
		close;
	}
	if(isbegin_quest(18138) == 0){
		mes "[Mimirgand]";
		mes "Oh, aren't you the new priest?";
		mes "Come and sit here with us.";
		mes "Let me tell you a story.";
		next;
		mes "[Mimirgand]";
		mes "In these harsh environment, there's a special way for us to survive.";
		mes "We are the ones who inherited the mana of the great Jormungandr.";
		next;
		mes "[Mimirgand]";
		mes "Do you know what that means?";
		next;
		select("It's the source of our lives!");
		mes "[Mimirgand]";
		mes "Listen carefully.";
		mes "In the past, our ancestors ruled the Midgard continent on the south with this mana.";
		next;
		mes "[Mimirgand]";
		mes "But Jormungandr was sealed by the evil human beings.";
		mes "But even at that moment, Jormungandr left his body for us.";
		next;
		mes "[Mimirgand]";
		mes "Yeah. That's what Isgard is.";
		mes "We took root from the blood of Lasgand, the last Rgan that Jormungandr protected.";
		mes "You were born from the mana that Jormungandr left behind.";
		next;
		mes "[Mimirgand]";
		mes "However, the others were not able to receive the same grace, so they were born with a different body.";
		mes "Do you know what that is?";
		next;
		select("Low-grade Rgans?");
		mes "[Mimirgand]";
		mes "Oh, You were a priest.";
		mes "Of course you'd know it.";
		mes "Yes, those who do not receive the grace of mana are abandoned and deemed as sinners.";
		next;
		mes "[Mimirgand]";
		mes "They desire the mana of others in very ugly way, and lust for the mana of the higher-grade ones.";
		mes "Every Rgan has a longing for mana, but they can't tell whats different from them.";
		next;
		mes "[Mimirgand]";
		mes "I can't let those low-grade desire what little mana we have left.";
		mes "I'll leave you with one important task.";
		next;
		mes "[Mimirgand]";
		mes "It's about retrieving the mana of Jormungandr that those sinners possess.";
		mes "The mana that we left behind is limited.";
		mes "They don't deserve to possess it.";
		next;
		mes "[Mimirgand]";
		mes "The mana is limited, we can't waste anymore mana, if we want to fullfill the will of Jormungandr and Lasgand.";
		next;
		mes "[Mimirgand]";
		mes "Recovering the mana from those sinners and use it again for the future of the Rgans.";
		mes "That's what we will do";
		next;
		mes "[Mimirgand]";
		mes "And for the sake of childrens born with grace like you, we must recover those mana.";
		next;
		select("Aren't they our kin?");
		mes "[Mimirgand]";
		mes "I don't see them the same as us.";
		mes "They are sinful.";
		mes "They  are also sacrifices.";
		mes "It's only natural for us to recover those mana from them who did not receive those grace.";
		next;
		mes "[Mimirgand]";
		mes "This is how we survive in this land of ice .";
		mes "Then, young priest.";
		mes "Go and punish those sinners and recover their mana.";
		setquest 18138;
		close;
	}
	if(isbegin_quest(18138) == 1){
		if(checkquest(18138,HUNTING) < 2 || countitem(1000707) < 10){
			mes "[Mimirgand]";
			mes "Where are the sinners?";
			mes "Young priest. didn't you say that you were born in the lower hatchery?";
			next;
			mes "[Mimirgand]";
			mes "What I mean is that that place is filled with those low-grades.";
			mes "Those ^4D4DFFPrimitive Rgan and Lowest-Rgan^000000 falls to the category of sinners.";
			next;
			mes "[Mimirgand]";
			mes "You job is to deal with them and bring back recover 10 ^4D4DFFLow-Grade Mana Core^000000";
			close;
		} else {
			mes "[Mimirgand]";
			mes "Looks like you've done the job.";
			mes "Come here.";
			mes "The mana core needs to be returned to the circulation.";
			next;
			mes "[Mimirgand]";
			mes "Don't you feel stronger after absorbing the mana core?";
			mes "Things like this can happen.";
			mes "Do you think it exist there?";
			next;
			select("Do you mean the low-grade Rgans?");
			mes "[Mimirgand]";
			mes "Yes. Sometimes there are Low-Grade Rgans with intelligence";
			mes "They are doing their jobs and try to wash away their sins.";
			next;
			mes "[Mimirgand]";
			mes "But that doesn't mean that a low-grade can absorb another low-grade and become an intermediate one .";
			mes "In any case, the size of each mana vessel is born from grace is different.";
			next;
			mes "[Mimirgand]";
			mes "Throwing rocks into the ocean wouldn't make a difference, right?";
			mes "But cutting mountains into a small pond will have results.";
			next;
			mes "[Mimirgand]";
			mes "Their only instinct is to covet the remaining mana, and sometimes they eat each other to fill up their mana, eventually it will fill their bowl.";
			next;
			mes "[Mimirgand]";
			mes "But size of that bowl won't change.";
			mes "It's the difference of what was naturally given by grace.";
			next;
			mes "[Mimirgand]";
			mes "But I don't know if Lasgand is trying to save those things";
			mes "If I can increase the given grace then I have nothing more to ask.";
			next;
			select("What do you need the mana core for?");
			mes "[Mimirgand]";
			mes "What does it means to be alive?";
			mes "Yes. You need a form.";
			next;
			mes "[Mimirgand]";
			mes "We use this is our form.";
			mes "Some are used in Lasgand's research, and the rest is used to restore exhausted mana.";
			next;
			mes "[Mimirgand]";
			mes "If you ever felt that your mana has exhausted too, absorb a mana core.";
			mes "It will make you feel better.";
			next;
			mes "[Mimirgand]";
			mes "Anyway, thank you for helping me with my work.";
			mes "This is also a job of a priest.";
			mes "Good job.";
			delitem 1000707,10;
			completequest 18138;
			questinfo_refresh;
			getitem 1000608,10;
			close;
		}
		end;
	}
	switch(checkquest(18140,PLAYTIME)){
		case -1:
			break;
			
		case 0:
		case 1:
			mes "[Mimirgand]";
			mes "Young priest.";
			mes "You have a duty to lead another believer.";
			mes "You should slowly look around the nest and find your own way to do things.";
			close;
			
		case 2:
			erasequest 18140;
			end;
	}
	if(isbegin_quest(18139) == 0){
		mes "[Mimirgand]";
		mes "Young priest.";
		mes "Won't you devote yourself for the teachings today?";
		mes "I mean is to do the sacred duty of labor.";
		next;
		if(select("What should I do?:I'm going to pray.") == 2){
			mes "[Mimirgand]";
			mes "Pray?";
			mes "Are you serious?";
			mes "Are you going to confess your faith with a pure heart?";
			mes "You better not be lying.";
			close;
		}
		mes "[Mimirgand]";
		mes "Deal with the bugs crawling around the snake nest and recover their mana.";
		mes "We won't be able to do any work while hungry.";
		next;
		mes "[Mimirgand]";
		mes "It's our job anyways.";
		mes "Someone has to do it.";
		mes "So go ahead.";
		setquest 18139;
		close;
	} else {
		if(countitem(1000707) < 30){
			mes "[Mimirgand]";
			mes "Recover 30 ^4D4DFFRgan Lower-Grade Mana Core^000000";
			mes "It's our sacred duty.";
			close;
		}
		mes "[Mimirgand]";
		mes "You've collected enough food for our daily necessity.";
		mes "This is blood, sweat, and flesh of other Rgans.";
		mes "It will be with our blood, flesh, and soul of another Rgan.";
		next;
		mes "[Mimirgand]";
		mes "Their sin will be cleansed and reborn with new blood.";
		mes "You did good today.";
		delitem 1000707,30;
		erasequest 18139;
		setquest 18140;
		getitem 1000608,2 + (getreputation("Ice Castle") >= 3000 ? 2 : 0);
		close;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17624) == 2 && isbegin_quest(18138) == 0";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"checkquest(18138,HUNTING) == 2 && countitem(1000707) >= 10";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(18138) == 2 && checkquest(18140,PLAYTIME) == -1 && isbegin_quest(18139) == 0";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(18138) == 2 && checkquest(18140,PLAYTIME) == 2 && isbegin_quest(18139) == 0";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(18139) == 1 && countitem(1000707) >= 30";
end;
}

jor_nest,37,75,1	script	Beruberugand#ep19r	4_EP19_RGAN_R1,{
	if(!191_scroll_check()){
		mes "[Rgan Bishop]"; //TODO GET ORIGINAL DIALOGUE
		mes "Humans shouldn't come here. Get out of this place.";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	if(isbegin_quest(18138) < 2){
		mes "[Beruberugand]";
		mes "Oh? Young Priest!";
		mes "You're the protagonist of the rumors, rumor about you waking up in the lower hatchery, is it true?";
		next;
		mes "[Beruberugand]";
		mes "What are you doing looking around?";
		mes "Mimirgand is looking for you.";
		close;
	}
	switch(checkquest(18142,PLAYTIME)){
		case -1:
			break;
			
		case 0:
		case 1:
			mes "[Beruberugand]";
			mes "Hmm. What should I make?";
			mes "A belt? Shoes? Perhaps a norigae?";
			mes "Oh, a headgear also would be nice.";
			mes "I'll give it to Lasgand if the it ended up well.";
			next;
			mes "[Beruberugand]";
			mes "If you have enough materials, would you like to make one too?";
			mes "Oh, I'm busy. I'm busy.";
			close;
			
		case 2:
			erasequest 18142;
			end;
	}
	if(isbegin_quest(18141) == 0){
		mes "[Beruberugand]";
		mes "Hey, can you help me if you have some time?";
		mes "I'm trying to make some ornaments, but the materials is hard to get.";
		next;
		mes "[Beruberugand]";
		mes "It's good that those non-believers brought some, but I like the traditional ones better.";
		mes "I mean handcrafted ones.";
		next;
		mes "[Beruberugand]";
		mes "I also made Mimirgand's ornaments. Did you see it?";
		mes "Jormungandr's scale clothes are already pretty, but it will be more gorgeous with coral decorations.";
		next;
		mes "[Beruberugand]";
		mes "But, there materials that you can get here is limited.";
		mes "So you ened to collect materials outside to make it more colorful.";
		mes "Can you help me?";
		next;
		if(select("Should I go now?:Sorry, I'm busy right now.") == 2){
			mes "[Beruberugand]";
			mes "Let me know if you're going out to drink in the glacier or plain area.";
			mes "I have a favor to ask of you.";
			close;
		}
		mes "[Beruberugand]";
		mes "Alright! Then, here's all the materials I need to make a delicate ornament!";
		mes "You can get them all in the plain and glaciers.";
		next;
		mes "[Beruberugand]";
		mes "10 Calapy from Calamaring";
		mes "10 Fluorescent Liquid from Limacina.";
		mes "10 Decoy Bell Flower from Unfrost Flower.";
		mes "10 Thin Tough Shell from Ice Straw.";
		next;
		mes "[Beruberugand]";
		mes "Do you remember it all? Don't forget it, okay?";
		mes "I'm going to prepare the scales, so make sure you get it!";
		setquest 18141;
		close;
	} else {
		if(countitem(1000829) < 10 || countitem(1000830) < 10 || countitem(1000824) < 10 || countitem(7326) < 10){
			mes "[Beruberugand]";
			mes "Well? Do you need me to mention the materials again? Then, listen carefully.";
			mes "10 Calapy from Calamaring";
			mes "10 Fluorescent Liquid from Limacina.";
			mes "10 Decoy Bell Flower from Unfrost Flower.";
			mes "10 Thin Tough Shell from Ice Straw.";
			next;
			mes "[Beruberugand]";
			mes "It's something that can be seen in both the glaciers and in the plains.";
			mes "Is it hard?";
			mes "I think you're strong enough. If you help me, I'll give you some petals from a snow flower that I've collected.";
			mes "Isn't it tempting?";
		} else {
			mes "[Beruberugand]";
			mes "Glazed Calapies!";
			mes "Fluorescent Liquid for the dyes!";
			mes "Such round Bell Flowers!";
			mes "These skins are so good for knitting...!";
			next;
			mes "[Beruberugand]";
			mes "Great, it's great!";
			mes "As expected, aren't you a generous one?";
			next;
			mes "[Beruberugand]";
			mes "Now, remember the deal?";
			mes "This is the reward for your troubles.";
			mes "If you have time, I'd like to ask you again next time.";
			delitem 1000829,10;
			delitem 1000830,10;
			delitem 1000824,10;
			delitem 7326,10;
			erasequest 18141;
			setquest 18142;
			getitem 1000608,2 + (getreputation("Ice Castle") >= 3000 ? 2 : 0);
		}
		close;
	}
	end;
	
OnInit:
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(18138) == 2 && isbegin_quest(18141) == 0 && checkquest(18142,PLAYTIME) == -1";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(18138) == 2 && isbegin_quest(18141) == 0 && checkquest(18142,PLAYTIME) == 2";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(18141) == 1 && countitem(1000829) >= 10 && countitem(1000830) >= 10 && countitem(1000824) >= 10 && countitem(7326) >= 10";
end;
}

jor_nest,257,26,5	script	Hanadosaragand#ep19r	21529,{
	if(!191_scroll_check()){
		mes "[Hanadosaragand]";
		mes "Kyaaaaah!!!!!!!!";
		mes "A human!! In the workplace!! It's here!!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	if(isbegin_quest(17624) < 2){
		mes "[Hanadosaragand]";
		mes "I wonder if that thing is still alive...";
		mes "I mean it was bitten by that thing...";
		mes "Oh...?";
		close;
	}
	switch(checkquest(18144,PLAYTIME)){
		case -1:
			break;
			
		case 0:
		case 1:
			mes "[Hanadosaragand]";
			mes "I'm going to dig ice. I'm going.";
			mes "Is it safe? It's safe, right?";
			mes "I'm going to dig ice.";
			close;
			
		case 2:
			erasequest 18144;
			end;
	}
	if(isbegin_quest(18143) == 0){
		mes "[Hanadosaragand]";
		mes "Priest, priest";
		mes "Call for help.. it's urgent. Help me.";
		mes "I'm so scared.";
		next;
		if(select("What's going on?:Look for another priest.") == 2){
			mes "[Hanadosaragand]";
			mes "Alright.";
			mes "Another priest. Another priest...";
			close;
		}
		mes "[Hanadosaragand]";
		mes "The Limacinas in front of the cave entrance is scary.";
		mes "Help me. Priest.";
		mes "The Calamarings are also scary. Priest.";
		next;
		mes "[Hanadosaragand]";
		mes "Help this poor worker to dig some ice.";
		mes "Those scary Limacinas. and Calamarings.. Please get rid of them.";
		next;
		select("You want me to get rid of monster outside the cave?");
		mes "[Hanadosaragand]";
		mes "That's right. That's right.";
		mes "The worker digging for ice. Both died.";
		mes "Now they want me to dig it.";
		mes "Help me. Priest.";
		next;
		if(select("Okay.:Look for another priest.") == 2){
			mes "[Hanadosaragand]";
			mes "Alright.";
			mes "Another priest. Another priest...";
			close;
		}
		mes "[Hanadosaragand]";
		mes "Then, please clean up the front of the cave.";
		mes "Please. I have to go dig for ice.";
		mes "Please. Priest.";
		setquest 18143;
		close;
	}
	if(isbegin_quest(18143) == 1){
		if(checkquest(18143,HUNTING) < 2){
			mes "[Hanadosaragand]";
			mes "The Limacinas in front of the cave entrance is scary.";
			mes "Help me. Priest.";
			mes "The Calamarings are also scary. Priest.";
			next;
			mes "[Hanadosaragand]";
			mes "Help this poor worker to dig up ice.";
			mes "Those scary Limacinas. and Calamarings.. Please get rid of them.";
		} else {
			mes "[Hanadosaragand]";
			mes "Can I now?";
			mes "Priest. Can I go to dig some ice?";
			mes "Thank you. Thank you.";
			mes "I'll give you all of these. Thank you.";
			erasequest 18143;
			setquest 18144;
			getitem 1000608,2 + (getreputation("Ice Castle") >= 3000 ? 2 : 0);
		}
		close;
	}
	end;
	
OnInit:
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(17624) == 2 && isbegin_quest(18143) == 0 && checkquest(18144,PLAYTIME) == -1";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(17624) == 2 && isbegin_quest(18143) == 0 && checkquest(18144,PLAYTIME) == 2";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"checkquest(18143,HUNTING) == 2";
end;
}

jor_nest,235,57,3	script	Pabuyasabigand#1	21529,{
	if(!191_scroll_check()){
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	if(isbegin_quest(17624) < 2){
		mes "[Pabuyasabigand]";
		mes "...";
		close;
	}
	if(isbegin_quest(8790) == 0){
		mes "[Pabuyasabigand]";
		mes "...";
		mes "...";
		mes "...?";
		mes "It's my first time seeing you priest.";
		mes "I'm Pabuyasabigand.";
		next;
		mes "[Pabuyasabigand]";
		mes "I'm a direct subordinate of Lasgand.";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "Direct?";
		next;
		mes "[Pabuyasabigand]";
		mes "I've never seen you before, but...";
		next;
		mes "[Pabuyasabigand]";
		mes "Ganurijamigand here.";
		mes "Sumobanugand who went to work... and a few other direct subordinates. Yeah.";
		mes "We collect shells and offer them to Lasgand. Yeah.";
		next;
		mes "[Ganurijamigand]";
		mes "Yes. We are doing a very helpful job. Yeah.";
		mes "Although we are only intermediate...";
		next;
		mes "Durgand suddenly became gloomy.";
		next;
		mes "[Pabuyasabigand]";
		mes "Recently... our work is getting harder.";
		mes "It became more difficult to collect shells because of the humans that came in the area where we are working.";
		next;
		select("There's a hatchery here too.");
		mes "[" + strcharinfo(0) + "]";
		mes "There's a hatchery here too.";
		next;
		mes "[Ganurijamigand]";
		mes "We're not in charge of hatching.";
		mes "Our job is to retrieve the shells from the warmth of the snake god.";
		next;
		mes "[Ganurijamigand]";
		mes "But, there are humans coming and out on the path that we've worked hard making on our own.";
		mes "But as you can see, we are just Rgans.";
		mes "There's no way we can stop it... I don't have anymore strength to make more roads.";
		next;
		mes "[Ganurijamigand]";
		mes "I've never seen you Priest before, but you look so strong.";
		mes "Would you...";
		mes "Would you...";
		next;
		mes "[Pabuyasabigand]";
		mes "collect... it's for Lasgand... just a few ... <ITEM>[shells]<INFO>1000822</INFO></ITEM>";
		mes "about 30...";
		next;
		mes "I don't know if you're shameless or brave.";
		next;
		if(select("I'll share you some.:Refuse") == 2){
			mes "[Pabuyasabigand]";
			mes "You were so cool and shiny that I asked...";
			mes "Just forget what you've heard, yeah.";
			mes "Lasgand... I'm sorry...";
			close;
		}
		mes "[Pabuyasabigand]";
		mes "Thank you very much!";
		mes "We'll be waiting here near the entrance.";
		setquest 8790;
		close;
	}
	if(isbegin_quest(8790) == 1){
		if(countitem(1000822) < 30){
			mes "[Pabuyasabigand]";
			mes "<ITEM>[Shell]<INFO>1000822</INFO></ITEM>... I need it...";
			mes "Priest...";
			mes "About 30...";
			close;
		}
		mes "[Pabuyasabigand]";
		mes "Priest!";
		mes "Did you really collect those shells?";
		mes "For us?";
		next;
		mes "[Pabuyasabigand]";
		mes "I can't thank you enough!";
		mes "May Jormungandr bless you!";
		mes "Thank you very much!";
		mes "I might need it again tomorrow...";
		mes "I'll think about it tomorrow, yeah!";
		npctalk "Thank you very much!","Pabuyasabigand#1",bc_self;
		delitem 1000822,30;
		completequest 8790;
		setquest 8791;
		getitem 1000608,10;
		close;
	}
	switch(checkquest(8791,PLAYTIME)){
		case -1:
			break;
			
		case 0:
		case 1:
			mes "[Pabuyasabigand]";
			mes "Priest, thank you for the shell that you shared earlier!";
			mes "We won't eat it so don't worry!";
			close;
			
		case 2:
			erasequest 8791;
			end;
	}
	if(isbegin_quest(8792) == 0){
		mes "[Pabuyasabigand]";
		mes "collect... it's for Lasgand... just a few ... <ITEM>[shells]<INFO>1000822</INFO></ITEM>";
		mes "about 30...";
		next;
		mes "I don't know if you're shameless or brave.";
		next;
		if(select("I'll share you some.:Refuse") == 2){
			mes "[Pabuyasabigand]";
			mes "You were so cool and shiny that I asked...";
			mes "Just forget what you've heard, yeah.";
			mes "Lasgand... I'm sorry...";
			close;
		}
		mes "[Pabuyasabigand]";
		mes "Thank you very much!";
		mes "We'll be waiting here near the entrance.";
		setquest 8792;
		close;
	}
	if(isbegin_quest(8792) == 1){
		if(countitem(1000822) < 30){
			mes "[Pabuyasabigand]";
			mes "<ITEM>[Shell]<INFO>1000822</INFO></ITEM>... I need it...";
			mes "Priest...";
			mes "About 30...";
			close;
		}
		mes "[Pabuyasabigand]";
		mes "Priest!";
		mes "Did you really collect those shells?";
		mes "For us?";
		next;
		mes "[Pabuyasabigand]";
		mes "I can't thank you enough!";
		mes "May Jormungandr bless you!";
		mes "Thank you very much!";
		mes "I might need it again tomorrow...";
		mes "I'll think about it tomorrow, yeah!";
		npctalk "Thank you very much!","Pabuyasabigand#1",bc_self;
		delitem 1000822,30;
		erasequest 8792;
		setquest 8791;
		getitem 1000608,2 + (getreputation("Ice Castle") >= 3000 ? 2 : 0);
		close;
	}
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17624) == 2 && isbegin_quest(8790) == 0";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(8790) == 1 && countitem(1000822) >= 30";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(8790) == 2 && checkquest(8791,PLAYTIME) == -1 && isbegin_quest(8792) == 0";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(8790) == 2 && checkquest(8791,PLAYTIME) == 2 && isbegin_quest(8792) == 0";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(8792) == 1 && countitem(1000822) >= 30";
end;
}

jor_nest,232,55,7	script	Ganurijamigand#1	21529,{
	if(!191_scroll_check()){
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	mes "[Ganurijamigand]";
	mes "I still don't know what that is.";
	mes "Yes.";
	close;
}

jor_nest,239,244,5	script	Sidhurgand#1	4_EP19_RGAN_SR3,{
	if(!191_scroll_check()){ //TODO
		mes "[Rgan Priest]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	if(isbegin_quest(17624) < 2){
		mes "It would be bad if you got caught while talking to any Rgan by mistake.";
		mes "I'll keep an eye out for a bit.";
		close;
	}
	if(isbegin_quest(8796) == 0){
		mes "[Sidhurgand]";
		mes "Woah!";
		mes "It's the first I've seen you!";
		mes "Woah~ your scale. It shines so brightly~";
		mes "I've never seen such clean scales~";
		next;
		mes "[Sidhurgand]";
		mes "I like you.";
		mes "Are you from the hatchery?";
		mes "I haven't seen any good eggs for a while~";
		mes "Then suddenly a very shiny one comes out~";
		next;
		mes "[Sidhurgand]";
		mes "I guess I still have a long way to go~";
		mes "I'm Sidhurgand.";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "I'm Senekiogand.";
		next;
		mes "[Sidhurgand]";
		mes "Can you give me some scales?";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "Huh??";
		next;
		mes "[Sidhurgand]";
		mes "Priest, don't worry.";
		mes "I might be ranked highly, but I won't ask you something reckless...";
		mes "Oh, how about some blood?";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "What??";
		next;
		mes "[Sidhurgand]";
		mes "You can't... Then, there's nothing we can do.";
		mes "Hm, I won't get away with it anyway... can you do it? How about the shell?";
		next;
		mes "[Sidhurgand]";
		mes "Ohh... don't make such face.";
		mes "You're cute, but wrinkles are forming on your forehead.";
		next;
		mes "[Sidhurgand]";
		mes "Not good to pass~.";
		next;
		select("Ask if he got one before");
		mes "[" + strcharinfo(0) + "]";
		mes "...for the scales...";
		mes "...do you have... other kins?";
		next;
		mes "[Sidhurgand]";
		mes "No!";
		mes "Sigh, why would you even ask that!";
		next;
		mes "[Sidhurgand]";
		mes "I said that I'd exchange it for my own scales, but they didn't want to!";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "(Of course they won't...)";
		next;
		mes "[Sidhurgand]";
		mes "Even though I haven't been out there for a while.";
		mes "I know that our people are at a very bad situation!";
		next;
		mes "[Sidhurgand]";
		mes "It's funny to say this to you, to someone who was just born.";
		mes "*Shhhh*...";
		next;
		mes "His mouth hissed like a crawling snake.";
		mes "He seems to be smiling.";
		next;
		mes "[Sidhurgand]";
		mes "You'll know what I mean when you go out. It's cold!";
		mes "To seek revenge in such a barren place, isn't it amazing?";
		next;
		mes "[Sidhurgand]";
		mes "Then we should do all that can be done!";
		mes "To increase my powers... and the other believers...";
		mes "You! Yes, won't you work under me?";
		next;
		mes "[Sidhurgand]";
		mes "Those old people told me to wait, don't get scales, and blood...";
		mes "That's why we are not progressing!";
		next;
		mes "[Sidhurgand]";
		mes "Don't you have any spare scales?";
		next;
		select("I don't have any");
		mes "[" + strcharinfo(0) + "]";
		mes "...I don't have.";
		next;
		mes "[Sidhurgand]";
		mes "It won't help if it's just a piece of scale.";
		mes "Then, head out and catch me 20 Primitive Rgans, you can pretend that you're going to just see them.";
		next;
		mes "[Sidhurgand]";
		mes "I want you to tell me how it feel.";
		mes "*Shhhh*...";
		next;
		if(select("Accept.:I don't want to do it.") == 2){
			mes "[" + strcharinfo(0) + "]";
			mes "...I don't want to.";
			next;
			mes "[Sidhurgand]";
			mes "I can't do much!";
			mes "This, excitement!";
			mes "Then... atleast a piece of your scale...";
			mes "...Eh, where are you going?";
			mes "Oy!";
			close;
		}
		mes "[Sidhurgand]";
		mes "Yeah, it would be better than being nagged by the old ones!";
		mes "Be careful!";
		setquest 8796;
		close;
	}
	if(isbegin_quest(8796) == 1){
		if(checkquest(8796,HUNTING) < 2){
			mes "[Sidhurgand]";
			mes "Please catch me 20 ^0000FFPrimitive Rgan^000000 and tell me how you feel about the difference.";
			close;
		}
		mes "[Sidhurgand]";
		mes "Oh, you're here?";
		next;
		mes "[Sidhurgand]";
		mes "Someone told me that there are many animals that are killed by their own kin.";
		mes "I have never been out of this cold continent before so I don't really know...";
		mes "I just heard that humans are the same.";
		next;
		mes "[Sidhurgand]";
		mes "Aren't they like that? We have to save our resources.";
		mes "So, did you catch a primitive Rgan?";
		mes "Did they feel like the same race as you?";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "...yes.";
		next;
		mes "[Sidhurgand]";
		mes "Right? So did I.";
		mes "And if they disappear and be left with a tiny mana core, then why are we born?";
		mes "If you ask that old one named Berunagand who preaches the doctrine, I wonder if he'll teach you?";
		completequest 8796;
		setquest 8797;
		getitem 1000608,10;
		next;
		mes "[Sidhurgand]";
		mes "See you again tomorrow!";
		mes "Let's build our friendship and forces again tomorrow.";
		close;
	}
	switch(checkquest(8797,PLAYTIME)){
		case -1:
			break;
			
		case 0:
		case 1:
			mes "[Sidhurgand]";
			mes "See you again tomorrow!";
			mes "Let's build our friendship and forces again tomorrow.";
			close;
			
		case 2:
			erasequest 8797;
			end;
	}
	if(isbegin_quest(8798) == 0){
		mes "[Sidhurgand]";
		mes "Is there anything missing from these?";
		mes "Scales?";
		mes "...maybe?";
		next;
		mes "[Sidhurgand]";
		mes "Then, how about catching 20 ^0000FFPrimitive Rgan^000000?";
		next;
		if(select("Accept.:I don't want to.") == 2){
			mes "[Sidhurgand]";
			mes "Why, do you have something to do?";
			mes "Are you going somewhere?";
			mes "Come on, you are on my side, right?";
			mes "Hey, hey!";
			mes "Don't go! Play with me!";
			close;
		}
		mes "[Sidhurgand]";
		mes "Yeah, it would be better than being nagged by the old ones!";
		mes "Be careful!";
		setquest 8798;
		close;
	}
	if(isbegin_quest(8798) == 1){
		if(checkquest(8798,HUNTING) < 2){
			mes "[Sidhurgand]";
			mes "Go out there.";
			mes "And catch 20 ^0000FFPrimitive Rgan^000000.";
			close;
		}
		mes "[Sidhurgand]";
		mes "Then, let's have a discussion about the feeling of hunting your own kin...";
		mes "What, don't make such a blatant hateful face!!";
		mes "...then, play with me instead!";
		erasequest 8798;
		setquest 8797;
		getitem 1000608,2 + (getreputation("Ice Castle") >= 3000 ? 2 : 0);
		close;
	}
	end;

OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17624) == 2 && isbegin_quest(8796) == 0";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"checkquest(8796,HUNTING) == 2";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(8796) == 2 && checkquest(8797,PLAYTIME) == -1 && isbegin_quest(8798) == 0";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(8796) == 2 && checkquest(8797,PLAYTIME) == 2 && isbegin_quest(8798) == 0";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"checkquest(8798,HUNTING) == 2";
end;
}

jor_nest,239,239,3	script	Delpanagand#1	4_EP19_RGAN_SR1,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	if(isbegin_quest(17624) < 2){
		mes "It would be bad if you got caught while talking to any Rgan by mistake.";
		mes "I'll keep an eye out for a bit.";
		close;
	}
	if(isbegin_quest(8793) == 0){
		mes "[Delpanagand]";
		mes "Ho, I've never seen a priest like you before, where did you come from? How did you get here?";
		mes "who's that, did you just crawl out of the hatchery?";
		mes "Who was managing it?";
		next;
		mes "[Delpanagand]";
		mes "...Yes?";
		mes "Oh, you're not from the hatchery?";
		mes "What. Why didn't you just tell me?";
		next;
		mes "A high-grade Rgan caught me as I was passing by and started talking to himself.";
		next;
		mes "[Delpanagand]";
		mes "Oh, I'm Delpanagand.";
		mes "Every Rgan who can talk knows me.";
		next;
		mes "[Delpanagand]";
		mes "Are you curious about anything?";
		next;
		select("There is.:Not really.");
		mes "[Delpanagand]";
		mes "You're an honest young one.";
		mes "But since you were just born, listen to me first.";
		next;
		mes "[Delpanagand]";
		mes "Well...";
		mes "Usually, I gather a number of our people and teach them at one time, trying to explain to only you is a bit strange to me.";
		next;
		mes "[Delpanagand]";
		mes "We are a race that protects this land under the protection of the great serpent Jormungandr. We are born from an egg without any parents, and the one raised us is the mana of this land.";
		mes "While all other living were born from a parent, we were born out of a will of the survivor, Lasgand and we took shape with the power of the great serpent Jormungandr.";
		next;
		mes "[Delpanagand]";
		mes "All praise, to the great serpent Jormungandr.";
		mes "And Lasgand, the survivor.";
		mes "Lasgand was blessed with such intelligence the moment he woke up from his egg.";
		mes "The blood of the strong makes strong eggs.";
		next;
		mes "[Delpanagand]";
		mes "All of the Rgans today are individuals that descended from Lasgand's blood.";
		mes "Although we don't have th econcept of parent and child, we have become to what we are because of the blood sown by Lasgand, inheriting the will of our ancestors, and gave us such power.";
		next;
		mes "[Delpanagand]";
		mes "The best out of all of us is Lasgand.";
		mes "Below him, is bishops like me who's a superior Rgan.";
		next;
		mes "[Delpanagand]";
		mes "You were born a priest at the mercy of our god.";
		mes "Those who take care of the eggs over there are called laymen.";
		mes "They woke up even before abosrbing the mana of god that could have been given to them, doing this job will make them useful...";
		next;
		mes "[Delpanagand]";
		mes "Not with just resources.";
		mes "We need to use our wisdom to help Lasgand and expand the Jormungandr cult throughout the whole continent.";
		mes "So are you curious about anything?";
		next;
		select("There is.:Not really, but.");
		mes "From Delpanagand's story, it seems that Rgans are born from eggs with a certain amount of intelligence even from the beginning, such as the ability to talk.";
		next;
		mes "I'm glad that some mistakes from the way I speak will be ignored.";
		next;
		mes "Why do we need to hear the story of Jormungandr?";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "There is.";
		next;
		mes "[Delpanagand]";
		mes "You're smart. Tell me.";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "Why do we have to learn words one by one when we are born?";
		next;
		mes "[Delpanagand]";
		mes "Oh, this is hard, I can't believe you started with such a hard question!";
		mes "But there's a little misunderstanding.";
		mes "We are not born to learn to speak.";
		next;
		mes "[Delpanagand]";
		mes "We are different individuals that came from Lasgand, but we Rgans have something in common, it's Lasgand's blood.";
		mes "There are natural things that would make us Rgans.";
		mes "Tell me what you know.";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "...";
		next;
		select("Rgan eats Rgans.");
		mes "[" + strcharinfo(0) + "]";
		mes "Rgan eats Rgans...";
		next;
		mes "[Delpanagand]";
		mes "Okay. There are things you don't have to learn. We were born knowing everything already because of Lasgand.";
		mes "The amount of doctrines may change as our power grow and our population grow.";
		next;
		mes "[Delpanagand]";
		mes "Well, I guess you've already heard a long story about Berunagand?";
		next;
		setquest 8793;
		mes "[Delpanagand]";
		mes "Then, I'll give you a teast.";
		mes "Go around the hatchery, and talk to the laymens who are protecting the eggs.";
		mes "<NAVI>[Obedient Child]<INFO>jor_nest,277,257,0,101,0</INFO></NAVI>,";
		mes "<NAVI>[Lazy Child]<INFO>jor_nest,220,279,0,101,0</INFO></NAVI>,";
		mes "<NAVI>[Unruly Child]<INFO>jor_nest,274,199,0,101,0</INFO></NAVI>, Would this be enough?";
		close;
	}
	if(isbegin_quest(8793) == 1){
		if(isbegin_quest(8801) < 1){
			mes "Go around the hatchery, and talk to the laymens who are protecting the eggs.";
			mes "<NAVI>[Obedient Child]<INFO>jor_nest,277,257,0,101,0</INFO></NAVI>,";
			mes "<NAVI>[Lazy Child]<INFO>jor_nest,220,279,0,101,0</INFO></NAVI>,";
			mes "<NAVI>[Unruly Child]<INFO>jor_nest,274,199,0,101,0</INFO></NAVI>, Would this be enough?";
			close;
		}
		mes "[Delpanagand]";
		mes "How was it? Like you... is there a person who's curious about something?";
		mes "Yo must have realized it now. It's about what each individual can do.";
		mes "You have to keep wondering, use your strength for the teaching.";
		completequest 8801;
		completequest 8793;
		getitem 1000608,10;
		next;
		mes "[Delpanagand]";
		mes "You have to do what you can.";
		close;
	}
	mes "[Delpanagand]";
	mes "You have to keep wondering, use your strength for the teaching.";
	end;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17624) == 2 && isbegin_quest(8793) == 0";
end;
}

jor_nest,277,257,1	script	Yamurisunigand#1	21529,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	if(isbegin_quest(8793) == 0){
		mes "It would be bad if you got caught while talking to any Rgan by mistake.";
		mes "I'll keep an eye out for a bit.";
		close;
	}
	if(isbegin_quest(8793) == 1 && isbegin_quest(8799) == 0){
		mes "[Yamurisunigand]";
		mes "You were asked by Delpanagand?";
		mes "To ask us what we think?";
		next;
		mes "[Yamurisunigand]";
		mes "...I don't know what to say, yeah.";
		next;
		select("How about your favorite thing");
		mes "[Yamurisunigand]";
		mes "Favorite thing?";
		mes "Warp place...";
		mes "I don't like being outside, yeah.";
		next;
		mes "I don't think I'll hear an important story.";
		setquest 8799;
		setquest 8802;
		completequest 8802;
		questinfo_refresh;
		mes "I'll have to look for another Rgan.";
		close;
	}
	if(isbegin_quest(8799) == 1){
		mes "[Yamurisunigand]";
		mes "Favorite thing?";
		mes "Warp place...";
		mes "I don't like being outside, yeah.";
		close;
	}

	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(8793) == 1 && isbegin_quest(8799) == 0";
end;
}

jor_nest,274,199,5	script	Gibedeusagand#1	21529,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	if(isbegin_quest(8799) == 0){
		mes "It would be bad if you got caught while talking to any Rgan by mistake.";
		mes "I'll keep an eye out for a bit.";
		close;
	}
	if(isbegin_quest(8799) == 1){
		mes "[Gibedeusagand]";
		mes "Oh, Hello!";
		mes "I was... surprised, yeah.";
		mes "I've never seen a priest like you before.";
		mes "Yeah.";
		mes "I'm not playing, yeah.";
		next;
		mes "[Gibedeusagand]";
		mes "...I, was taking care of this, yeah.";
		next;
		select("Do you have any complaints");
		mes "[Gibedeusagand]";
		mes "Complaints?";
		mes "...No priest has said anything like that to me before.";
		next;
		mes "[Gibedeusagand]";
		mes "We are... the believers of god, yeah.";
		mes "...";
		mes "Working is normal, right?";
		mes "I want to play, but...";
		mes "There is work that needs to be done.";
		next;
		mes "I don't think I'll hear an important story.";
		completequest 8799;
		setquest 8800;
		setquest 8804;
		completequest 8804;
		questinfo_refresh;
		mes "I'll have to look for another Rgan.";
		close;
	}
	if(isbegin_quest(8800) == 1){
		mes "... you have another thing to do, yeah?";
		close;
	}

	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(8799) == 1";
end;
}

jor_nest,220,279,1	script	Tagimirodigand#1	21529,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	if(isbegin_quest(8800) == 0){
		mes "It would be bad if you got caught while talking to any Rgan by mistake.";
		mes "I'll keep an eye out for a bit.";
		close;
	}
	if(isbegin_quest(8800) == 1){
		mes "[Tagimirodigand]";
		mes "You were asked by Delpanagand?";
		mes "To ask us what we think?";
		next;
		mes "[Tagimirodigand]";
		mes "I don't want to work... Yeah.";
		next;
		select("What do you want to do");
		mes "[Tagimirodigand]";
		mes "What I want to do?";
		mes "To lie on the floor... in the place where the mana is warm...";
		mes "I want to sleep, yeah.";
		mes "But I have to work for the revival of Jormungandr.";
		next;
		mes "I don't think there's any deep thought.";
		completequest 8800;
		setquest 8801;
		setquest 8803;
		completequest 8803;
		questinfo_refresh;
		mes "I need to report back to Delpanagand.";
		close;
	}
	mes "[Tagimirodigand]";
	mes "I want to sleep... in a warm place.";
	close;

OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(8800) == 1";
end;
}

jor_nest,17,97,5	script	Berunagand#1	4_EP19_RGAN_SR2,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	if(isbegin_quest(17624) < 2){
		mes "It would be bad if you got caught while talking to any Rgan by mistake.";
		mes "I'll keep an eye out for a bit.";
		close;
	}
	if(isbegin_quest(8795) == 0){
		mes "A group of Rgan are sitting around and listening to a high-grade Rgan's preaching in a calm atmosphere.";
		next;
		if(select("Listen to them.:Just pass them by.") == 2){
			mes "Let's look somewhere else.";
			close;
		}
		mes "[Berunagand]";
		mes "...Bit I, Berunagand thinks of things a little differently.";
		mes "As all of the believers here know, our one and only god is Jormungandr.";
		next;
		mes "[Berunagand]";
		mes "Everyone here is with the god's blessing.";
		mes "We are bound to be different from other races. We have no parents, nor children, but we formed a community in the name of god's will.";
		npctalk "Are you a newborn? He's preaching right now, so listen quietly.","Handimarigand#1",bc_self;
		next;
		mes "The high-grade, who called himself Berunagand, looked at my way and nodded.";
		next;
		mes "[Berunagand]";
		mes "Looks like there's a new member. Let's welcome him.";
		npctalk "Welcome!","Handimarigand#1",bc_self;
		npctalk "Just sit down.","Yuramigand#1",bc_self;
		npctalk "...","Olragusudagand#1",bc_self;
		next;
		mes "[Berunagand]";
		mes "Okay, okay. Everyone be quiet again.";
		mes "I was going to wrap up my sermon today, but now that we have the new priest, let's talk a little bit more.";
		next;
		mes "[Berunagand]";
		mes "It's been a while... shall we talk about Lasgand?";
		next;
		mes "[Berunagand]";
		mes "He's not participating recently because he's busy right now, for the revival of the Jormungandr cult, Lasgand has traveled all over this cold land.";
		mes "He spared no expense and use his blood to create new people in the grace of god.";
		next;
		mes "[Berunagand]";
		mes "Thhe final descendants of our forefathers, the mana of Lasgand's blood, and the mana of this land full of god's grace, has began to take on a new life.";
		mes "But everything didn't go his way. The first things that came out of the egg...";
		next;
		mes "[Berunagand]";
		mes "we're Primitive Rgans who were not chosen by god.";
		npctalk "We are sorry...","Olragusudagand#1",bc_self;
		next;
		mes "[Berunagand]";
		mes "Lasgand turned them into mana cores on by one out of pity.";
		mes "And after a long time... Now, during that time, I have also been with Lasgand...";
		next;
		mes "[Berunagand]";
		mes "After many years of repeating of sowing his blood and reaping them, I had this thought.";
		next;
		mes "[Berunagand]";
		mes "All of the Rgans today, regardless of rank, are a part and a descendants of Lasgand.";
		mes "No parents, nor children.";
		next;
		mes "[Berunagand]";
		mes "Why do they appear different from each outcome?";
		mes "That is... I think, it depends on how far you want to reach your desires.";
		next;
		mes "[Berunagand]";
		mes "We don't know the pain that they've been through.";
		mes "And the priest here for example, can't know what we've been through for a long time.";
		next;
		mes "[Berunagand]";
		mes "Lasgand was immortalized by a curse involving humans, but...";
		mes "If Lasgand's energy declines before our revenge on the humans, our race will head into ruins.";
		next;
		mes "[Berunagand]";
		mes "...revenge against humans!";
		npctalk "Long live Jormungandr!","Handimarigand#1",bc_self;
		npctalk "Long live Jormungandr!","Olragusudagand#1",bc_self;
		next;
		mes "[Berunagand]";
		mes "For Our ancestors Grudge!";
		npctalk "Long live Lasgand!","Handimarigand#1",bc_self;
		npctalk "Long live Lasgand!","Olragusudagand#1",bc_self;
		next;
		mes "[Berunagand]";
		mes "Before it's too late... shouldn't we reorganize our powers?";
		next;
		mes "[Berunagand]";
		mes "Before it's too late...";
		mes "We should be all ready.";
		next;
		mes "[Berunagand]";
		mes "That's it for today's sermon.";
		mes "Relax for a little while everyone.";
		npctalk "I have to get back to work...","Olragusudagand#1",bc_self;
		npctalk "...","Yuramigand#1",bc_self;
		next;
		mes "[Yuramigand]";
		mes "Let me see you for a moment.";
		npctalk "Shh. Don't look this way.","Yuramigand#1",bc_self;
		setquest 8795;
		questinfo_refresh;
		close;
	}
	mes "[Berunagand]";
	mes "That's it for today's sermon.";
	mes "See you next time, new priest.";
	close;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17624) == 2 && isbegin_quest(8795) == 0";
end;
}

jor_nest,18,92,7	script	Yuramigand#1	4_EP19_RGAN_SR1,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	if(isbegin_quest(17624) < 2){
		mes "It would be bad if you got caught while talking to any Rgan by mistake.";
		mes "I'll keep an eye out for a bit.";
		close;
	}
	if(isbegin_quest(8795) == 0){
		mes "[Yuramigand]";
		mes "...";
		close;
	}
	if(isbegin_quest(8795) == 1){
		mes "[Yuramigand]";
		mes "Let me see you for a moment.";
		npctalk "Shh. Don't look this way.","Yuramigand#1",bc_self;
		next;
		select("Yes?:May I help you?");
		mes "[Yuramigand]";
		mes "...";
		mes "I just heard this as soon as I was born, I know it's very stimulating.";
		mes "You don't have to listen to it.";
		next;
		mes "[Yuramigand]";
		mes "I don't know what Berunagand is up to.";
		mes "He talks like that, but I'm not really sure if he's acting for Lasgand.";
		next;
		mes "[Yuramigand]";
		mes "Did you understand?";
		next;
		select("Yes:I dont know");
		mes "[Yuramigand]";
		mes "...though only time will tell.";
		mes "Don't get too close to Berunagand.";
		mes "Lasgand is very unhappy with it.";
		next;
		select("Is this true?:I understand");
		mes "[Yuramigand]";
		mes "...";
		mes "...";
		next;
		mes "[Yuramigand]";
		mes "Berunagand, loyalty to Lasgand is real though.";
		next;
		mes "[Yuramigand]";
		mes "...That's right. As Alreigand said, why is everyone born out of Lasgand's blood are so different?";
		completequest 8795;
		getitem 1000608,10;
		close;
	}
	mes "[Yuramigand]";
	mes "Be careful not to be targeted by someone.";
	close;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(8795) == 1";
end;
}

jor_nest,24,97,3	script	Handimarigand#1	4_EP19_RGAN_R1,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	mes "[Handimarigand]";
	mes "The sermons are nice and narrative.";
	close;
}

jor_nest,23,91,1	script	Olragusudagand#1	21529,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	mes "[Olragusudagand]";
	mes "...";
	mes "I'm doing. Work.";
	mes "Hard. Atonement.";
	close;
}

jor_nest,21,99,3	script	Rakanorugand#1	4_EP19_RGAN_R2,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	mes "[Rakanorugand]";
	mes "Follow the teachings of god...";
	close;
}

jor_nest,74,100,5	script	Alreigand#1	4_EP19_RGAN_SR3,{
	if(!191_scroll_check()){
		mes "[Rgan Bishop]"; //TODO GET ORIGINAL DIALOGUE
		mes "Humans shouldn't come here. Get out of this place.";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	if(isbegin_quest(17624) < 2){
		mes "It would be bad if you got caught while talking to any Rgan by mistake.";
		mes "I'll keep an eye out for a bit.";
		close;
	}
	if(isbegin_quest(8794) == 0){
		mes "[Alreigand]";
		mes "...?";
		mes "You.";
		mes "I have never seen you before. Come over here.";
		next;
		mes "Before he ended his sentence, the superior Rgan grabbed my forearm, pulled it and put his teeth in!";
		mes "Did I get caught!?";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "Ouch!";
		next;
		mes "[Alreigand]";
		mes "Tsk. How dare you.";
		mes "It didn't bite, It didn't bite.";
		mes "I wasn't able to eat.";
		next;
		mes "[Alreigand]";
		mes "Am I exaggerating?";
		mes "You're not a coward. Oh, aren't you afraid of me?";
		next;
		select("I'm afraid.:I'm not afraid.");
		mes "[Alreigand]";
		mes "The answer is...";
		mes "I didn't eat at all.";
		next;
		if(select("Why are you doing this:Run away quickly...") == 2){
			mes "[Alreigand]";
			mes "Haha! Coward!";
			close;
		}
		mes "[" + strcharinfo(0) + "]";
		mes "Why are you doing... this?";
		next;
		mes "[Alreigand]";
		mes "Are you a newborn? You don't look like one, but why are you talking like a worker?";
		mes "But a simple worker wouldn't even think of this. I guess you're smart.";
		next;
		mes "[Alreigand]";
		mes "We're all born under the same conditions, but why is our personalities and thoughts are different? Isn't it fun!";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "????";
		next;
		mes "[Alreigand]";
		mes "What. I played around too much.";
		mes "I can't believe you're this naive.";
		mes "Is it really possible for someone to eat an arm?";
		next;
		mes "[Alreigand]";
		mes "I mean~. I don't do it because Gumumigand told me not to.";
		mes "Do you know Gumumigand? Have you met him?";
		mes "No? Nothing I can do about that.";
		next;
		mes "[Alreigand]";
		mes "Anyway, I was thinking about how to eat a raw Primitive Rgan, and that's his idea.";
		mes "No matter what I do, nothing changes so why bother.";
		mes "Ohh, I'm speechless.";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "(This is ridiculous...)";
		next;
		mes "[Alreigand]";
		mes "No matter how much we rely on the land with the mana of the god... even if they were made with the ancestors of our blood and gained a body, they are different like other creatures.";
		mes "Can't we just simply absorb them without turning them into cores?";
		next;
		mes "[Alreigand]";
		mes "I heard that humans cook what they eat, right?";
		mes "I want to try that~!!";
		mes "The only thing I've never done in my life...";
		next;
		mes "[Alreigand]";
		mes "That's why Gumumigand...";
		mes "...then...";
		mes "That's...";
		next;
		mes "I just heard a very scary story.";
		mes "However, if Gumumigand in the story has the same principle of a regular Rgan, it seems that it won't eat a Primitive Rgan directly.";
		next;
		mes "[Alreigand]";
		mes "...so get about 15 <ITEM>[Rgan Low-Grade Mana Core]<INFO>1000707</INFO></ITEM>.";
		mes "If you don't have one, find one.";
		next;
		mes "[Alreigand]";
		mes "Go!";
		setquest 8794;
		close;
	}
	if(isbegin_quest(8794) == 1){
		if(countitem(1000707) < 15){
			mes "[Alreigand]";
			mes "...so get about 15 <ITEM>[Rgan Low-Grade Mana Core]<INFO>1000707</INFO></ITEM>.";
			mes "If you don't have one, find one.";
			close; 
		}
		mes "[Alreigand]";
		mes "Did you really bring it?";
		mes "This child is going to be big...";
		mes "How were you born?";
		mes "Whose blood were you born from? It's not me, right?";
		next;
		mes "[Alreigand]";
		mes "Seriously, woah... I feel sorry for him... Hey. Take this.";
		mes "Don't say anything because you're going to be naive again. Got it?";
		delitem 1000707,15;
		completequest 8794;
		getitem 1000608,10;
		close;
	}
	mes "[Alreigand]";
	mes "*Chewing*.";
	mes "Stay away from me.";
	mes "I'm in a bad mood.";
	close;
	
OnInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(17624) == 2 && isbegin_quest(8794) == 0";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(8794) == 1 && countitem(1000707) >= 15";
end;
}

jor_nest,273,23,3	script	Sharyararara#ep19iwin07	21529,{
	if(isbegin_quest(17624) < 2){
		mes "[Sharyarararagand]";
		mes "Just working here... Shalalalala~";
		close;
	}
	mes "[Sahryara]";
	mes "Hi! It's me, Sharyara";
	mes "Are you going back to the Ice Castle?";
	mes "Care to join me?";
	next;
	if(select("I still have stuffs to do.:Let's go back to the Ice Castle!") == 1){
		mes "[Sahryara]";
		mes "Okay.";
		mes "Let's go together later.";
		mes "I'll keep traveling back and forth there.";
		close;
	}
	mes "[Sahryara]";
	mes "Do you think I'm the fastest Iwin in the Ice Castle?";
	mes "I'll help you not fall on the ice, so let's go!";
	close2;
	warp "icecastle",55,120;
	end;
	
OnInit:
	questinfo QTYPE_CLICKME,QMARK_YELLOW,"isbegin_quest(17624) == 2";
end;
}

jor_nest,253,75,7	script	Mirarasimigand#1	21529,{
	if(!191_scroll_check()){
		mes "[Rgan Bishop]"; //TODO GET ORIGINAL DIALOGUE
		mes "Humans shouldn't come here. Get out of this place.";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	mes "[Mirarasimigand]";
	mes "...";
	mes "*Snore*.";
	mes "I worked hard today. I should take a break...";
	mes "I hope Jormungandr will forgive me.";
	mes "*Snore*...";
	next;
	mes "He was sleeping against the wall with a tired face even though he is a Rgan.";
	close;
}

jor_nest,219,38,5	script	Rgan Priest#e19ms104	4_EP19_RGAN_R3,{
	if(!191_scroll_check()){
		mes "[Rgan Priest]";
		mes "How did a human who should be in the lower floors was able to come here?";
		close2;
		warp "jor_back3",65,321;
		end;	
	}
	npctalk "Huh? I didn't see...? Oh, a newborn??","Rgan Priest#e19ms104",bc_self;
	end;
}

jor_nest,76,22,3	script	Rgan Bishop#e19ms100	4_EP19_RGAN_SR1,{
	if(!191_scroll_check()){
		mes "[Rgan Bishop]"; //TODO GET ORIGINAL DIALOGUE
		mes "Humans shouldn't come here. Get out of this place.";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "Ho~ Are you the newborn?","Rgan Bishop#e19ms100",bc_self;
	sleep2 1500;
	npctalk "I see, have you decided on your career path?","Rgan Bishop#e19ms101",bc_self;
	sleep2 1500;
	npctalk "No, you can't talk about that to a newborn! It's not something you should talk about this early.","Rgan Bishop#e19ms100",bc_self;
	end;
}

jor_nest,73,17,7	script	Rgan Bishop#e19ms101	4_EP19_RGAN_SR2,{
	if(!191_scroll_check()){
		mes "[Rgan Bishop]"; //TODO GET ORIGINAL DIALOGUE
		mes "Humans shouldn't come here. Get out of this place.";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "We always welcome children.","Rgan Bishop#e19ms101",bc_self;
	npctalk "You can always ask if you don't know something, got it?","Rgan Bishop#e19ms100",bc_self;
	end;
}

jor_nest,108,79,5	script	Rgan Priest#e19ms100	4_EP19_RGAN_R2,{
	if(!191_scroll_check()){ //TODO
		mes "[Rgan Priest]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "Please treat me well... only me...","Rgan Priest#e19ms100",bc_self;
	end;
}

jor_nest,152,108,5	script	Biranimalgand#1	4_EP19_RGAN_R1,{
	if(!191_scroll_check()){ //TODO
		mes "[Rgan Priest]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	mes "[Biranimalgand]";
	mes "I worked hard for Rasugand today too. Phew.";
	close;
}

jor_nest,123,132,5	script	Rgan Priest#e19ms103	4_EP19_RGAN_R1,{
	if(!191_scroll_check()){ //TODO
		mes "[Rgan Priest]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "Are you the newborn? I hope you'll be able to grow up and work with me.","Rgan Priest#e19ms103",bc_self;
	end;
}

jor_nest,224,207,3	script	Heart Hunter#e19ms100	EP19_MD_HEARTHUNTER_AT,{
	if(!191_scroll_check()){
		mes "[Heart Hunter]";
		mes "Laborers are not allowed to come here.";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "Halt- don't come any closer.","Heart Hunter#e19ms100",bc_self;
	end;
}

jor_nest,231,253,5	script	Heart Hunter#e19ms101	EP19_MD_HEARTHUNTER_AT,{
	if(!191_scroll_check()){
		mes "[Heart Hunter]";
		mes "Laborers are not allowed to come here.";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "Kid, take a step back.","Heart Hunter#e19ms101",bc_self;
	end;
}

jor_nest,231,186,5	script	Heart Hunter#e19ms102	EP19_MD_HEARTHUNTER_AT,{
	if(!191_scroll_check()){
		mes "[Heart Hunter]";
		mes "Laborers are not allowed to come here.";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "Rgan, go away-","Heart Hunter#e19ms102",bc_self;
	end;
}

jor_nest,232,161,3	script	Heart Hunter#e19ms103	EP19_MD_HEARTHUNTER_AT,{
	if(!191_scroll_check()){
		mes "[Heart Hunter]";
		mes "Laborers are not allowed to come here.";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "Why? Do you have business?","Heart Hunter#e19ms103",bc_self;
	end;
}

jor_nest,217,144,5	script	Heart Hunter#e19ms104	EP19_MD_HEARTHUNTER_AT,{
	if(!191_scroll_check()){
		mes "[Heart Hunter]";
		mes "Laborers are not allowed to come here.";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "Oh~ You can't come here~ Didn't you hear?","Heart Hunter#e19ms104",bc_self;
	end;
}

jor_nest,239,143,3	script	Heart Hunter#e19ms105	EP19_MD_HEARTHUNTER_AT,{
	if(!191_scroll_check()){
		mes "[Heart Hunter]";
		mes "Laborers are not allowed to come here.";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "If you come any closer, you won't be able to see any good things, do you want that?","Heart Hunter#e19ms105",bc_self;
	end;
}

jor_nest,190,203,5	script	Heart Hunter#e19ms106	EP19_MD_HEARTHUNTER_AT,{
	if(!191_scroll_check()){
		mes "[Heart Hunter]";
		mes "Laborers are not allowed to come here.";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "This is a very important facility, what if it breaks because of you? So, don't come here!","Heart Hunter#e19ms106",bc_self;
	end;
}

jor_nest,109,184,5	script	Heart Hunter#e19ms107	EP19_MD_HEARTHUNTER_AT,{
	if(!191_scroll_check()){
		mes "[Heart Hunter]";
		mes "Laborers are not allowed to come here.";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "Don't wander around here. This place is not for you to go.","Heart Hunter#e19ms107",bc_self;
	end;
}

jor_nest,80,150,1	script	Heart Hunter#e19ms108	EP19_MD_HEARTHUNTER_AT,{
	if(!191_scroll_check()){
		mes "[Heart Hunter]";
		mes "Laborers are not allowed to come here.";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "Didn't you see what's written there? It says. Turn around and leave.","Heart Hunter#e19ms108",bc_self;
	end;
}

jor_nest,24,169,5	script	Heart Hunter#e19ms109	EP19_MD_HEARTHUNTER_AT,{
	if(!191_scroll_check()){
		mes "[Heart Hunter]";
		mes "Laborers are not allowed to come here.";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "Oh, are you a staff? You're not. Then you shouldn't be here.","Heart Hunter#e19ms109",bc_self;
	end;
}

jor_nest,82,163,5	script	Chime#e19ms71	4_SCR_MT_ROBOTS,5,5,{
	npctalk "Ding- Dong-","Chime#e19ms71",bc_self;
	end;
	
OnTouch:
	if(isbegin_quest(17625) > 0 && isbegin_quest(17626) < 2){
		npctalk "Someone entered the lab.","",bc_self;
		unittalk getcharid(3),strcharinfo(0) + " : This place is well decorated.",bc_self;
	}
end;
}

jor_nest,29,251,5	script	Rgan Security#e19ms91	4_EP19_RGAN_R1,5,5,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "......","Rgan Security#e19ms91",bc_self;
	end;
	
OnTouch:
	if(isbegin_quest(17649)){
		cloaknpc("Rgan Security#e19ms91",true,getcharid(0));
		cloaknpc("Rgan Security#e19ms92",true,getcharid(0));
	}
end;
}

jor_nest,23,247,5	script	Rgan Security#e19ms92	4_EP19_RGAN_R2,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "......","Rgan Security#e19ms92",bc_self;
	end;
}

jor_nest,41,270,3	script	Rgan Security#e19ms93	4_EP19_RGAN_SR1,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "......","Rgan Security#e19ms93",bc_self;
	end;
}

jor_nest,35,270,5	script	Rgan Security#e19ms94	4_EP19_RGAN_SR2,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "......","Rgan Security#e19ms94",bc_self;
	end;
}

jor_nest,115,212,3	script	Rgan Priest#e19ms101	4_EP19_RGAN_R1,{
	if(!191_scroll_check()){ //TODO
		mes "[Rgan Priest]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "Aren't you a newborn? Where are you going?","Rgan Priest#e19ms101",bc_self;
	end;
}

jor_nest,179,144,3	script	Rgan Bishop#e19ms102	4_EP19_RGAN_SR2,{
	if(!191_scroll_check()){
		mes "[Rgan Bishop]"; //TODO GET ORIGINAL DIALOGUE
		mes "Humans shouldn't come here. Get out of this place.";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "I have never seen you before? Oh, is it not?? Sorry, I'm not good at remembering faces.","Rgan Bishop#e19ms102",bc_self;
	end;
}
	
jor_nest,272,196,5	script	Mirediyamilgand#1	21529,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	mes "[Mirediyamilgand]";
	mes "I'm working. I'm a great worker.";
	close;
}

jor_nest,243,223,3	script	Lacrusgand#1	4_EP19_RGAN_R3,{
	if(!191_scroll_check()){
		mes "[Rgan Priest]"; //TODO GET ORIGINAL DIALOGUE
		mes "Humans shouldn't come here. Get out of this place.";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	mes "[Lacrusgand]";
	mes "If I play a trick on someone. It'll be all arranged by god.";
	next;
	mes "[Lacrusgand]";
	mes "They won't be able to tell who did it.";
	close;
}

jor_nest,241,223,5	script	Dungidungigand#1	4_EP19_RGAN_R1,{
	if(!191_scroll_check()){
		mes "[Rgan Priest]"; //TODO GET ORIGINAL DIALOGUE
		mes "Humans shouldn't come here. Get out of this place.";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	mes "[Dungidungigand]";
	mes "I think there are childrens who are playing tricks, but I can't tell which one.";
	close;
}

jor_nest,261,233,3	script	Iganenegand#1	4_EP19_RGAN_R3,{
	if(!191_scroll_check()){
		mes "[Rgan Priest]"; //TODO GET ORIGINAL DIALOGUE
		mes "Humans shouldn't come here. Get out of this place.";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	mes "[Iganenegand]";
	mes "You have never seen a priest?";
	mes "May we be together under god's grace in the future.";
	close;
}

jor_nest,278,219,5	script	Wayunalgosulgand#1	21529,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	mes "[Wayunalgosulgand]";
	mes "I work. I'm a wonderful worker.";
	mes "I don't know what's good about being wonderful, but it's cool.";
	close;
}

jor_nest,272,257,7	script	Hanukorimagand#1	21529,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	mes "[Hanukorimagand]";
	mes "I'm working. I'm a great worker.";
	close;
}

jor_nest,251,279,7	script	Kaesubiramigand#1	21529,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	mes "[Kaesubiramigand]";
	mes "I'm Working. Wherever I go there is work to do.";
	close;
}

jor_nest,186,173,3	script	Rgan Priest#e19ms102	4_EP19_RGAN_R3,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Priest]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "Wow!! It's my first time seeing a newborn! If you come to me later, I'll tell you all about the doctrine!","Rgan Priest#e19ms102",bc_self;
	end;
}

jor_que,67,168,5	script	Rgan#i19ms100	21529,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "I was about to report that my work is done... Where did everyone go...?","Rgan#i19ms100",bc_self;
	end;
}

jor_que,156,191,3	script	Rgan#i19ms101	21529,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "Woah... Everyone's busy... I'm getting dizzy.","Rgan#i19ms101",bc_self;
	end;
}

jor_que,204,237,3	script	Rgan#i19ms102	21529,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "Great... one. Where... they go...?","Rgan#i19ms102",bc_self;
	end;
}

jor_que,81,77,5	script	Rgan#i19ms103	21529,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "You're... a priest... here with me... I'd better go---","Rgan#i19ms103",bc_self;
	end;
}

jor_que,208,140,1	script	Rgan#i19ms104	21529,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "Is there no food today?","Rgan#i19ms104",bc_self;
	end;
}

jor_que,184,184,7	script	Rgan#i19ms105	21529,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "Why is it quiet? Why is it quiet today? Is it quiet? Why is it quiet?","Rgan#i19ms105",bc_self;
	end;
}

jor_que,128,81,7	script	Rgan#i19ms106	21529,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "Grow well eggs. I'm glad it didn't get attacked. Grow well.","Rgan#i19ms106",bc_self;
	end;
}

jor_que,173,195,3	script	Rgan#i19ms107	21529,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "Hurts.. it hurts... I want to go outside-","Rgan#i19ms107",bc_self;
	end;
}

jor_que,162,58,1	script	Rgan#i19ms108	21529,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "Ah, I'm not playing. Really- I am, telling the truth.","Rgan#i19ms108",bc_self;
	end;
}

jor_que,159,105,3	script	Rgan#i19ms109	21529,{
	if(!191_scroll_check()){ //TODO
		mes "[Intermediate Rgan]";
		mes "It's a human! A human is here!";
		close2;
		warp "jor_back3",65,321;
		end;
	}
	npctalk "Aghhh!!! Save me!!!","Rgan#i19ms109",bc_self;
	end;
}

jor_que,271,34,0	script	#i19ms100	HIDDEN_WARP_NPC,5,5,{
	end;
	
OnTouch:
	.@id = atoi(replacestr(strnpcinfo(2),"i19ms10",""));
	switch(.@id){
		case 0:
			cloaknpc("Human#i19ms100",false,getcharid(0));
			cloaknpc("Human#i19ms101",false,getcharid(0));
			setpcblock PCBLOCK_NPC,true;
			npctalk "Why... is the surveillance gone? Is it time for work?","Human#i19ms100",bc_self;
			sleep2 1000;
			npctalk "It hasn't been long since we ended our turn. Is this a trap? Will they punish us if we move?","Human#i19ms101",bc_self;
			sleep2 2500;
			npctalk "Why would they go for such a trouble?","Human#i19ms100",bc_self;
			npctalk "Maybe they're trying to get rid of us. I won't even dare run away.","Human#i19ms101",bc_self;
			cloaknpc("Iwin#i19ms100",false,getcharid(0));
			sleep2 1000;
			npctalk "There's one here! Everyone! Let's get out of here!","Iwin#i19ms100",bc_self;
			sleep2 1500;
			npctalk "Eh, what? Why?","Human#i19ms100",bc_self;
			npctalk "I knew it!!! This is a trap!","Human#i19ms101",bc_self;
			sleep2 1500;
			npctalk "You are free now. Let's get out of here.","Iwin#i19ms100",bc_self;
			sleep2 1500;
			npctalk "I don't know what's happening but I'm out of here! Even if it's a trap!","Human#i19ms100",bc_self;
			sleep2 500;
			cloaknpc("Human#i19ms100",true,getcharid(0));
			sleep2 500;
			npctalk "You should go now. We don't have much time.","Iwin#i19ms100",bc_self;
			npctalk "Argh, whatever!!","Human#i19ms101",bc_self;
			sleep2 500;
			cloaknpc("Human#i19ms101",true,getcharid(0));
			sleep2 500;
			setpcblock PCBLOCK_NPC,false;
			cloaknpc("Iwin#i19ms100",true,getcharid(0));
			end;
			
		case 1:
			cloaknpc("Human#i19ms102",false,getcharid(0));
			setpcblock PCBLOCK_NPC,true;
			npctalk "Should... I go out? Or not? Why is the surveillance gone, it's making me anxious...","Human#i19ms102",bc_self;
			sleep2 1500;
			cloaknpc("Iwin#i19ms101",false,getcharid(0));
			npctalk "Human, are you alone?","Iwin#i19ms101",bc_self;
			sleep2 1500;
			npctalk "Ahh!!! No, I never even thought about going out!","Human#i19ms102",bc_self;
			sleep2 1500;
			npctalk "You have to leave now when there is no surveillance. If there's no other person with you then let's go out now.","Iwin#i19ms101",bc_self;
			sleep2 1500;
			npctalk "Why all of a sudden...?","Human#i19ms102",bc_self;
			sleep2 1500;
			npctalk "We're rescuing all the humans here. Let's go out and talk about the details later.","Iwin#i19ms101",bc_self;
			sleep2 1500;
			npctalk "Let's do that!!! It's just me here, so we can leave now!","Human#i19ms102",bc_self;
			sleep2 500;
			setpcblock PCBLOCK_NPC,false;
			cloaknpc("Iwin#i19ms101",true,getcharid(0));
			cloaknpc("Human#i19ms102",true,getcharid(0));
			end;
			
		case 2:
			setpcblock PCBLOCK_NPC,true;
			cloaknpc("Human#i19ms103",false,getcharid(0));
			cloaknpc("Human#i19ms104",false,getcharid(0));
			npctalk "Are they really gone?","Human#i19ms104",bc_self;
			sleep2 1000;
			npctalk "I'll go and take a look.","Human#i19ms103",bc_self;
			sleep2 500;
			npctalk "Be careful...","Human#i19ms104",bc_self;
			sleep2 1000;
			cloaknpc("Iwin#i19ms102",false,getcharid(0));
			npctalk "Are there only the two of you? And I'm not a Rgan! I'm an Iwin in a disguise!","Iwin#i19ms102",bc_self;
			sleep2 1500;
			npctalk "Ahhhhhhhh!!!!!!!!!!","Human#i19ms104",bc_self;
			npctalk "Please be quiet. This is a surprise.","Human#i19ms103",bc_self;
			sleep2 1500;
			npctalk "Does that mean, we can leave this place now?","Human#i19ms103",bc_self;
			sleep2 1500;
			npctalk "Yes, please follow me. I'll take you somewhere safe.","Iwin#i19ms102",bc_self;
			sleep2 1500;
			npctalk "Hey, let's go. This is our chance.","Human#i19ms103",bc_self;
			sleep2 500;
			cloaknpc("Human#i19ms103",true,getcharid(0));
			npctalk "Hey! Don't just go following someone you don't know...!","Human#i19ms104",bc_self;
			sleep2 1500;
			npctalk "I'm telling the truth.","Iwin#i19ms102",bc_self;
			sleep2 1500;
			npctalk "Even if you say that...","Human#i19ms104",bc_self;
			sleep2 500;
			cloaknpc("Human#i19ms104",true,getcharid(0));
			cloaknpc("Iwin#i19ms102",true,getcharid(0));
			setpcblock PCBLOCK_NPC,false;
			end;
			
		case 3:
			setpcblock PCBLOCK_NPC,true;
			cloaknpc("Human#i19ms105",false,getcharid(0));
			npctalk "Won't it be hard if I go out now? I'll sneak out when everything calms down.","Human#i19ms105",bc_self;
			cloaknpc("Iwin#i19ms103",false,getcharid(0));
			sleep2 1000;
			npctalk "Here's another one...! Human, you have to leave now.","Iwin#i19ms103",bc_self;
			sleep2 1500;
			npctalk "What, what? Why is an intermediate Rgan is so polite?","Human#i19ms105",bc_self;
			sleep2 1500;
			npctalk "I'm not a Rgan. We have to go out for now, I'll explain the situation as we go.","Iwin#i19ms103",bc_self;
			sleep2 1500;
			npctalk "I'll take you to a safe place.","Iwin#i19ms103",bc_self;
			sleep2 1500;
			npctalk "I don't know what's happening, but I found out your lying to me, I won't stay still!","Human#i19ms105",bc_self;
			sleep2 1500;
			npctalk "Yes, yes. Make sure to bring your belongings. Let's go.","Iwin#i19ms103",bc_self;
			sleep2 500;
			cloaknpc("Iwin#i19ms103",true,getcharid(0));
			cloaknpc("Human#i19ms105",true,getcharid(0));
			setpcblock PCBLOCK_NPC,false;
			end;
			
		case 4:
			setpcblock PCBLOCK_NPC,true;
			cloaknpc("Human#i19ms106",false,getcharid(0));
			npctalk "I can hear footsteps... Please just walk by... How did you get here...","Human#i19ms106",bc_self;
			sleep2 1000;
			cloaknpc("Iwin#i19ms104",false,getcharid(0));
			npctalk "Someone is hiding here! Is there anyone else?","Iwin#i19ms104",bc_self;
			sleep2 1500;
			npctalk "Oh... sorry. I don't want to be punished. I don't want to die. Please, forgive me.","Human#i19ms106",bc_self;
			sleep2 1500;
			npctalk "Sigh... this is why I don't want to look like this. Human, I'm not a Rgan but an Iwin.","Iwin#i19ms104",bc_self;
			sleep2 1500;
			npctalk "Go outside with me, I won't hurt you. There's a safe place there.","Iwin#i19ms104",bc_self;
			sleep2 1500;
			npctalk "How can I believe that...","Human#i19ms106",bc_self;
			sleep2 1500;
			npctalk "Oh, sigh... we'll be in big trouble if we don't go now.","Iwin#i19ms104",bc_self;
			sleep2 1500;
			npctalk "Why is it important... Will I die just like this... No...","Human#i19ms106",bc_self;
			sleep2 1500;
			npctalk "Let's go.","Iwin#i19ms104",bc_self;
			sleep2 500;
			cloaknpc("Human#i19ms106",true,getcharid(0));
			cloaknpc("Iwin#i19ms104",true,getcharid(0));
			setpcblock PCBLOCK_NPC,false;
			end;
	}
end;
}

jor_que,255,240,0	duplicate(#i19ms100)	#i19ms101	HIDDEN_WARP_NPC,5,5
jor_que,222,275,0	duplicate(#i19ms100)	#i19ms102	HIDDEN_WARP_NPC,5,5
jor_que,187,269,0	duplicate(#i19ms100)	#i19ms103	HIDDEN_WARP_NPC,5,5
jor_que,121,39,0	duplicate(#i19ms100)	#i19ms104	HIDDEN_WARP_NPC,5,5

jor_que,277,34,3	duplicate(dummynpc)	Human#i19ms100	4_M_REPAIR
jor_que,276,33,7	duplicate(dummynpc)	Human#i19ms101	4_M_REPAIR
jor_que,271,34,5	duplicate(dummynpc)	Iwin#i19ms100	21529
jor_que,252,244,7	duplicate(dummynpc)	Human#i19ms102	4_M_ALCHE_A
jor_que,255,240,1	duplicate(dummynpc)	Iwin#i19ms101	21529
jor_que,227,277,3	duplicate(dummynpc)	Human#i19ms103	4_M_TELEPORTER
jor_que,223,277,3	duplicate(dummynpc)	Human#i19ms104	4_M_ORIENT02
jor_que,222,275,7	duplicate(dummynpc)	Iwin#i19ms102	21529
jor_que,183,272,5	duplicate(dummynpc)	Human#i19ms105	4_M_SITDOWN
jor_que,187,269,1	duplicate(dummynpc)	Iwin#i19ms103	21529
jor_que,125,41,5	duplicate(dummynpc)	Human#i19ms106	4_M_MASKMAN
jor_que,121,39,7	duplicate(dummynpc)	Iwin#i19ms104	21529

//= Monster Spawn
jor_tail,0,0	monster	Shining Seaweed	21524,80,5000,5000
jor_tail,0,0	monster	Ice Gangu	21525,90,5000,5000
jor_back1,0,0	monster	Ice Straw	21523,80,5000,5000
jor_back1,0,0	monster	Shining Seaweed	21524,60,5000,5000
jor_back1,0,0	monster	Ice Gangu	21525,60,5000,5000
jor_back2,0,0	monster	Ice Straw	21523,35,5000,5000
jor_back2,0,0	monster	Ice Gangu	21525,25,5000,5000
jor_back2,0,0	monster	Limacina	21520,70,5000,5000
jor_back2,0,0	monster	Unfrost Flower	21521,70,5000,5000
jor_back3,0,0	monster	Ice Gangu	21525,20,5000,5000
jor_back3,0,0	monster	Limacina	21520,70,5000,5000
jor_back3,0,0	monster	Calamaring	21522,70,5000,5000
jor_back3,0,0	monster	Primitive Rgan	21526,30,5000,5000
jor_back3,0,0	monster	Lowest Rgan	21527,15,5000,5000
jor_back3,0,0	boss_monster	Ultra Limacina	21537,1,14400000,7200000
jor_dun01,0,0	monster	Primitive Rgan	21526,110,5000,5000
jor_dun01,0,0	monster	Lowest Rgan	21527,60,5000,5000
jor_dun01,0,0	monster	Lesser Rgan	21528,20,5000,5000
jor_dun01,0,0	monster	Heart Hunter AT	21599,10,5000,5000
jor_dun02,0,0	monster	Lowest Rgan	21527,90,5000,5000
jor_dun02,0,0	monster	Lesser Rgan	21528,80,5000,5000
jor_dun02,0,0	monster	Intermediate Rgan	21529,20,5000,5000
jor_dun02,0,0	monster	Heart Hunter AT	21599,10,5000,5000
jor_ab01,0,0	monster	Cave Calamaring	21540,70,5000,5000
jor_ab01,0,0	monster	Cave Flower	21541,60,5000,5000
jor_ab01,0,0	monster	Discarded Primitive Rgan	21538,70,5000,5000
jor_ab01,0,0	monster	Baby Hallucigenia	21543,60,5000,5000
jor_ab01,0,0	monster	Modified Superior Rgan	21600,10,5000,5000
jor_ab02,0,0	monster	One-Eyed Dollocaris	21544,60,5000,5000
jor_ab02,0,0	monster	Two-Eyed Dollocaris	21545,60,5000,5000
jor_ab02,0,0	monster	Hallucigenia	21542,70,5000,5000
jor_ab02,0,0	monster	Entangled Intermediate Rgan	21601,10,5000,5000
jor_ab02,0,0	monster	Discarded Intermediate Rgan	21539,70,5000,5000

//= Warps
jor_tail,98,287,0	warp	to_jor_back1_1	2,2,jor_back1,93,13
jor_tail,134,281,0	warp	to_jor_back1_2	2,2,jor_back1,131,13
jor_tail,219,294,0	warp	to_jor_back1_3	2,2,jor_back1,222,22
jor_back1,98,10,0	warp	to_tail_1	2,2,jor_tail,98,284
jor_back1,131,10,0	warp	to_tail_2	2,2,jor_tail,139,280
jor_back1,222,19,0	warp	to_tail_3	2,2,jor_tail,219,291
jor_back1,385,229,0	warp	to_icecastle	2,2,icecastle,20,123
jor_back1,214,390,0	warp	to_jor_back2-1	2,2,jor_back2,229,17
jor_back2,229,12,0	warp	to_jor_back1-1	2,2,jor_back1,214,387
jor_back2,9,248,0	warp	to_jor_back3-1	2,2,jor_back3,364,245
jor_back3,368,245,0	warp	to_jor_back2-2	2,2,jor_back2,13,248
jor_back3,63,326,0	warp	to_jor_dun01-1	2,2,jor_dun01,113,10
jor_dun01,113,7,0	warp	to_jor_back3-2	2,2,jor_back3,65,321
jor_dun02,286,87,0	warp	to_jor_dun01-2	2,2,jor_dun01,12,238
jor_dun03,58,48,0	warp	hw_ep19re3	2,2,jor_nest,62,255
jor_nest,282,18,0	warp	to_jor_dun02	2,2,jor_dun02,21,32
jor_ab01,113,7,0	warp	to_jor_back2-3	2,2,jor_back2,261,313
jor_ab02,286,87,0	warp	to_jor_ab01	2,2,jor_ab01,12,238
icecastle,17,123,0	warp	to_jor_back1-2	2,2,jor_back1,380,229
icecastle,213,175,0	warp	to_icas_in	2,2,icas_in,138,187
icecastle,186,222,0	warp	in_barracks	2,2,icas_in,108,32
icecastle,124,171,0	warp	in_restaurant	2,2,icas_in,243,36
icecastle,62,223,0	warp	in_house2	2,2,icas_in,85,112
icecastle,88,241,0	warp	in_house3	2,2,icas_in,138,112
icecastle,159,243,0	warp	in_house4	2,2,icas_in,191,112
icecastle,62,137,0	warp	in_house5	2,2,icas_in,249,112
icas_in,138,183,0	warp	out_icas	2,2,icecastle,208,168
icas_in,85,257,0	warp	in_icas_room_L	2,2,icas_in,49,258
icas_in,53,258,0	warp	out_icas_room_L	2,2,icas_in,89,257
icas_in,193,257,0	warp	in_icas_room_R	2,2,icas_in,226,258
icas_in,222,258,0	warp	out_icas_room_R	2,2,icas_in,189,257
icas_in,83,233,0	warp	in_dining	2,2,icas_in,25,169
icas_in,25,166,0	warp	out_dining	2,2,icas_in,83,229
icas_in,193,233,0	warp	in_bedroom	2,2,icas_in,253,169
icas_in,253,166,0	warp	in_bedroom2	2,2,icas_in,193,229
icas_in,108,29,0	warp	out_barracks	2,2,icecastle,186,218
icas_in,92,60,0	warp	in_barracks_L	2,2,icas_in,56,60
icas_in,59,60,0	warp	out_barracks_L	2,2,icas_in,95,60
icas_in,125,60,0	warp	in_barracks_R	2,2,icas_in,161,60
icas_in,158,60,0	warp	out_barracks_R	2,2,icas_in,122,60
icas_in,243,33,0	warp	out_restaurant	2,2,icecastle,123,168
icas_in,33,109,0	warp	out_house1	2,2,icecastle,60,211
icas_in,85,109,0	warp	out_house2	2,2,icecastle,67,222
icas_in,138,109,0	warp	out_house3	2,2,icecastle,88,238
icas_in,191,109,0	warp	out_house4	2,2,icecastle,159,241
icas_in,249,109,0	warp	out_house5	2,2,icecastle,62,134

-	script	#191_init	-1,{
	end;
	
OnInit:
	cloaknpc("Miriam#ep19miriam01",true);
	cloaknpc("Maram#ep19maram01",true);
	cloaknpc("Suad#ep19suad01",true);
	cloaknpc("Smart Ellie#ep19elly01",true);
	cloaknpc("Lazy#ep19lazy01",true);
	cloaknpc("Crux#ep19crux01",true);
	cloaknpc("Miriam#ep19miriam02",true);
	cloaknpc("Maram#ep19maram02",true);
	cloaknpc("Crux#ep19crux02",true);
	cloaknpc("Lehar#ep19lehar01",true);
	cloaknpc("Lehar#ep19lehar02",true);
	cloaknpc("Nihil#ep19nh01",true);
	cloaknpc("Lehar#ep19lehar03",true);
	cloaknpc("Lazy#ep19lazy02",true);
	cloaknpc("Maram#ep19maram03",true);
	cloaknpc("Miriam#ep19miriam03",true);
	cloaknpc("Lazy#ep19lazyjt",true);
	cloaknpc("Maram#ep19maramjt",true);
	cloaknpc("Miriam#ep19miriamjt",true);
	cloaknpc("Lehar#ep19lehar04",true);
	cloaknpc("Lazy#ep19lazy03",true);
	cloaknpc("Maram#ep19maram04",true);
	cloaknpc("Miriam#ep19miriam04",true);
	cloaknpc("Vellgunde#ep19vell01",true);
	cloaknpc("Voglinde#ep19vog01",true);
	cloaknpc("Guardian Leon#ep19leon01",true);
	cloaknpc("Guardian Aurelie#ep19arl01",true);
	cloaknpc("Voglinde#ep19",true);
	cloaknpc("Maram#ep19",true);
	cloaknpc("Miriam#ep19",true);
	cloaknpc("Lehar#ep19",true);
	cloaknpc("Saintess#ep19",true);
	cloaknpc("Traveling Bag#ep19_2",true);
	cloaknpc("Traveling Bag#ep19_3",true);
	cloaknpc("Traveling Bag#ep19_4",true);
	cloaknpc("Traveling Bag#ep19_5",true);
	cloaknpc("Traveling Bag#ep19_6",true);
	cloaknpc("Du#ep19_cas",true);
	cloaknpc("Maggi#ep19_cas",true);
	cloaknpc("Alf#ep19_cas",true);
	cloaknpc("Mark#ep19_cas",true);
	cloaknpc("Ellis#ep19",true);
	cloaknpc("Lehar#ep19_3",true);
	cloaknpc("Friederike#ep19entrance",true);
	cloaknpc("Du#ep19entrance",true);
	cloaknpc("Mark#ep19entrance",true);
	cloaknpc("Maggi#ep19entrance",true);
	cloaknpc("Alf#ep19entrance",true);	
	cloaknpc("Lehar#ep19_room",true);
	cloaknpc("Syururu#ep19_room",true);
	cloaknpc("Tatarin#ep19_room",true);
	cloaknpc("Voglinde#ep19_room",true);
	cloaknpc("Du#ep19_room",true);
	cloaknpc("Mark#ep19_room",true);
	cloaknpc("Maggi#ep19_room",true);
	cloaknpc("Alf#ep19_room",true);
	cloaknpc("Alf#ep19_jor2",true);
	cloaknpc("Mark#ep19_jor2",true);
	cloaknpc("Alf#ep19_ab",true);
	cloaknpc("Mark#ep19_ab",true);
	cloaknpc("Friederike#ep19_ab",true);
	cloaknpc("Alf#ep19_jor2",true);
	cloaknpc("Mark#ep19_jor2",true);
	cloaknpc("Lehar#ep19_2",true);
	cloaknpc("Syururu#ep19_ab",true);
	cloaknpc("#iwsmember",true);
	cloaknpc("Mysterious Young Man#flunch",true);
	cloaknpc("Lunch#flunch",true);
	cloaknpc("Juncea#e19ms00",true);
	cloaknpc("Horuru#e19ms11",true);
	cloaknpc("Horuru#if19msdq",true);
	cloaknpc("Lazy#e19ms11",true);
	cloaknpc("Lehar#e19ms11",true);
	cloaknpc("Miriam#e19ms11",true);
	cloaknpc("Horuru#e19ms12",true);
	cloaknpc("Lazy#e19ms12",true);
	cloaknpc("Lehar#e19ms12",true);
	cloaknpc("Miriam#e19ms12",true);
	cloaknpc("Miriam#e19ms20",true);
	cloaknpc("Horuru#e19ms21",true);
	cloaknpc("Lazy#e19ms21",true);
	cloaknpc("Lehar#e19ms21",true);
	cloaknpc("Lazy#e19ms31",true);
	cloaknpc("Rgan#e19ms21",true);
	cloaknpc("Rgan#e19ms22",true);
	cloaknpc("Rgan Bishop#e19ms31",true);
	cloaknpc("Lazy#e19ms41",true);
	cloaknpc("Lazy#e19ms51",true);
	cloaknpc("Lazy#e19ms61",true);
	cloaknpc("Sarekgand#e19ms61",true);
	cloaknpc("Bagot#e19ms61",true);
	cloaknpc("Juncea#e19ms71",true);
	cloaknpc("Rgan Priest#e19ms82",true);
	cloaknpc("Horuru#e19ms81",true);
	cloaknpc("Lehar#e19ms81",true);
	cloaknpc("Rgan Bishop#e19ms81",true);
	cloaknpc("Bagot#e19ms71",true);
	cloaknpc("Lazy#e19ms72",true);
	cloaknpc("Sarekgand#e19ms71",true);
	cloaknpc("Lazy#e19ms71",true);
	cloaknpc("Lazy#e19ms91",true);
	cloaknpc("Lazy#e19ms92",true);
	cloaknpc("Miriam#e19ms91",true);
	cloaknpc("Lehar#e19ms91",true);
	cloaknpc("Horuru#e19ms91",true);
	cloaknpc("Lazy#e19ms93",true);
	cloaknpc("Lazy#ep19re1",true);
	cloaknpc("#warp_ep19re1",true);
	cloaknpc("Pile of Papers#ep19re1",true);
	cloaknpc("Pile of Papers#ep19re2",true);
	cloaknpc("Pile of Papers#ep19re3",true);
	cloaknpc("Pile of Papers#ep19re4",true);
	cloaknpc("Pile of Papers#ep19re5",true);
	cloaknpc("Yugurungand#ep19re2",true);
	cloaknpc("Rgan Guard#ep19re1",true);
	cloaknpc("Rgan Guard#ep19re2",true);
	cloaknpc("Jaerorong#ep19re2",true);
	cloaknpc("Wirorong#ep19re1",true);
	cloaknpc("Juncea#ep19re1",true);
	cloaknpc("Bagot#ep19re1",true);
	cloaknpc("Lazy#ep19re2",true);
	cloaknpc("Miriam#ep19re1",true);
	cloaknpc("Unnoticed Box#ep19re1",true);
	cloaknpc("Empty Test Tube#ep19re1",true);
	cloaknpc("Rgan#ep19re1",true);
	cloaknpc("Juncea#ep19re2",true);
	cloaknpc("Juncea#i19ms00",true);
	cloaknpc("Juncea#i19ms01",true);
	cloaknpc("Juncea#i19ms02",true);
	cloaknpc("Lazy#i19ms00",true);
	cloaknpc("Lazy#i19ms01",true);
	cloaknpc("Lehar#i19ms00",true);
	cloaknpc("Miriam#i19ms00",true);
	cloaknpc("Vellgunde#i19ms00",true);
	cloaknpc("Aurelie#i19ms00",true);
	cloaknpc("Iwin#i19ms00",true);
	cloaknpc("Iwin Recon Unit#epm00",true);
	cloaknpc("Horuru#i19ms11",true);
	cloaknpc("Miriam#i19ms11",true);
	cloaknpc("Lehar#i19ms11",true);
	cloaknpc("Lazy#i19ms11",true);
	cloaknpc("Iwin Recon#i19ms12",true);
	cloaknpc("Iwin Recon#i19ms13",true);
	cloaknpc("Iwin Recon#i19ms14",true);
	cloaknpc("Iwin Recon#i19ms15",true);
	cloaknpc("Human#i19ms91",true);
	cloaknpc("Human#i19ms92",true);
	cloaknpc("Human#i19ms93",true);
	cloaknpc("Lazy#i19ms12",true);
	cloaknpc("Miriam#i19ms12",true);
	cloaknpc("Lehar#i19ms12",true);
	cloaknpc("Lehar#i19ms13",true);
	cloaknpc("Human#i19ms94",true);
	cloaknpc("Human#i19ms95",true);
	cloaknpc("Human#i19ms96",true);
	cloaknpc("Horuru#i19ms31",true);
	cloaknpc("Iwin#i19ms31",true);
	cloaknpc("Iwin#i19ms32",true);
	cloaknpc("Iwin#i19ms33",true);
	cloaknpc("Lehar#i19ms21",true);
	cloaknpc("Lazy#i19ms21",true);
	cloaknpc("Horuru#i19ms21",true);
	cloaknpc("Iwin#i19ms21",true);
	cloaknpc("Iwin#i19ms22",true);
	cloaknpc("Leon#i19ms21",true);
	cloaknpc("Aurelie#i19ms21",true);
	cloaknpc("Voglinde#i19ms21",true);
	cloaknpc("Miriam#i19ms21",true);
	cloaknpc("Lazy#if19msdq",true);
	cloaknpc("Juncea#if19msdq",true);
	cloaknpc("Maram#ep19maram05",true);
	cloaknpc("Miriam#ep19miriam06",true);
	cloaknpc("Rescued Survivor#ep19_01",true);
	cloaknpc("Rescued Survivor#ep19_02",true);
	cloaknpc("Aroron#ep19re2",true);
end;
}