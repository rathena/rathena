# Convert passwords to MD5 hashes

UPDATE `login` SET `user_pass`=MD5(`user_pass`);
