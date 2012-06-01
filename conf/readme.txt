What is the import folder for?

Most people don't know the real use of the import folder. After you do, you will wonder 
what you ever did without it.

The main thing it does, is provide a way for you to change your config settings without
having to update the files every time you update your server. You store your changes, and 
the rest are updated with rAthena (usually though SVN).

How does this work?

Well, you place only the settings you have changed in the import files. I'll use 
battle_athena.conf and battle_conf.txt for my example. Everytime you update you conf 
folder, using the normal method, you have to go and edit the configs again. So, you have to
redo your rates, redo your ip addresses, you have to redo it all. Well, not with the import
system.

Say you want to change your base experience rate from the default (100)to 7x (700). Well 
then you would place this in your import/battle_conf.txt:

// Rate at which exp. is given. (Note 2)
base_exp_rate: 700

You don't need the comment (duh, it's a commnet), but I usually leave them for clarity 
sake.

So, now this new setting take place over the setting in battle_athena.conf. You just keep 
this file everytime you update, and your setting will always be there. Neat, isn't it?

So, yeah, that's what the import folder is for. I hope to see a lot more people use it, to 
make my life as a managed server runer better.

Semi-guide by Ajarn
