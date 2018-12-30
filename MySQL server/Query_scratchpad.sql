/*for deleting everything*/
DROP TABLE `SocialNetwork`.`InteractionLog`;
DROP TABLE `SocialNetwork`.`Notifications`;
DROP TABLE `SocialNetwork`.`Posts`;
DROP TABLE `SocialNetwork`.`Users`;

DROP DATABASE `SocialNetwork`;

CREATE DATABASE `SocialNetwork` /*!40100 DEFAULT CHARACTER SET latin1 */;

CREATE TABLE `SocialNetwork`.`Users` (
  `userID` smallint(5) unsigned NOT NULL AUTO_INCREMENT,
  `userName` varchar(25) NOT NULL,
  `passwordHash` varchar(25) NOT NULL,
  PRIMARY KEY (`userID`),
  UNIQUE KEY `userID_UNIQUE` (`userID`),
  UNIQUE KEY `userName_UNIQUE` (`userName`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `SocialNetwork`.`Posts` (
  `postID` smallint(5) unsigned NOT NULL AUTO_INCREMENT,
  `posterUserID` smallint(5) unsigned NOT NULL,
  `posteeUserID` smallint(5) unsigned NOT NULL,
  `timestamp` datetime(6) NOT NULL DEFAULT NOW(6),
  `content` TEXT NOT NULL,
  PRIMARY KEY (`postID`),
  UNIQUE KEY `postID_UNIQUE` (`postID`),
  KEY `fk_Posts_1_idx` (`posterUserID`),
  CONSTRAINT `fk_Posts_1` FOREIGN KEY (`posterUserID`) REFERENCES `Users` (`userID`) ON DELETE CASCADE ON UPDATE CASCADE,
  KEY `fk_Posts_2_idx` (`posteeUserID`),
  CONSTRAINT `fk_Posts_2` FOREIGN KEY (`posteeUserID`) REFERENCES `Users` (`userID`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `SocialNetwork`.`Notifications` (
  `notificationID` smallint(5) unsigned NOT NULL AUTO_INCREMENT,
  `postID` smallint(5) unsigned NOT NULL,
  `userID` smallint(5) unsigned NOT NULL,
  `readFlag` boolean NOT NULL DEFAULT 0,
  `timestamp` datetime(6) NOT NULL DEFAULT NOW(6),
  PRIMARY KEY (`notificationID`),
  UNIQUE KEY `notificationID_UNIQUE` (`notificationID`),
  UNIQUE KEY `post_user_UNIQUE` (`postID`,`userID`),
  KEY `fk_Notifications_1_idx` (`userID`),
  CONSTRAINT `fk_Notifications_1` FOREIGN KEY (`userID`) REFERENCES `Users` (`userID`) ON DELETE CASCADE ON UPDATE CASCADE,
  KEY `fk_Notifications_2_idx` (`postID`),
  CONSTRAINT `fk_Notifications_2` FOREIGN KEY (`postID`) REFERENCES `Posts` (`postID`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `SocialNetwork`.`InteractionLog` (
  `interactionID` smallint(5) unsigned NOT NULL AUTO_INCREMENT,
  `userID` smallint(5) unsigned NOT NULL,
  `sessionID` int unsigned,
  `timestamp` datetime(6) NOT NULL DEFAULT NOW(6),
  `logout` boolean NOT NULL,
  `socketDescriptor` smallint(5) unsigned NOT NULL,
  `command` varchar(50) NOT NULL,
  PRIMARY KEY (`interactionID`),
  UNIQUE KEY `interactionID_UNIQUE` (`interactionID`),
  KEY `fk_InteractionLog_1_idx` (`userID`),
  CONSTRAINT `fk_InteractionLog_1` FOREIGN KEY (`userID`) REFERENCES `Users` (`userID`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

insert into SocialNetwork.Users (userName, passwordHash)
values  ('alex', '17663506432727786073'), ('ben', '12927111708687947557'), 
		('cris', '11740314204215096121'), ('don', '12745502948907907845'), 
        ('eddy', '4771635686586901585'), ('fred', '3210639344949365877'), 
        ('george', '6136068120051800929'), ('honey', '9050044803222492725'),
        ('imy', '1091350801367770665'), ('jack', '7868285383349367941'),
        ('krish', '15963054882994457561'), ('lilly', '16906882082752180197'),
        ('mary', '18327857878878084177'), ('noah', '5069992954181438069'),
        ('omar', '1260992177983512433'), ('pretty', '10940044000550006709'),
        ('quinton', '4523305108125428409'), ('roger', '18264053755285864037'),
        ('sam', '9499914711864451609'), ('tom', '10229820929279828485');
