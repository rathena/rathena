What is the import folder for?
-------------------------------------------------------------------------------

The folder provides a way for you to change your config settings without having
to update the files every time you update your server. You store your changes,
and the rest is updated with rAthena.

How does this work?
-------------------------------------------------------------------------------

Place only the settings you have changed in the import files.
For example, if you want to change a value in /battle/exp.conf:

	// Rate at which exp. is given. (Note 2)
	base_exp_rate: 700

You could instead copy the setting into /import/battle_conf.txt,
and you'll eliminate any problems updating in the future.

Neat, isn't it?

- Semi-guide by Ajarn / Euphy