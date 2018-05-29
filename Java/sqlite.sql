CREATE TABLE `users` (
  `UID` INTEGER PRIMARY KEY NOT NULL,
  `PwdMd5` VARCHAR(32) NOT NULL,
  `PwdSalt` VARCHAR(32) NOT NULL,
  `Username` VARCHAR(32) NOT NULL,
  `Email` VARCHAR(50) NOT NULL,
  `JoinTime` DATETIME NOT NULL,
  UNIQUE(`Email`)
);

CREATE TABLE `login_history`(
  `id` INTEGER PRIMARY KEY NOT NULL,
  `UID` INTEGER NOT NULL,
  `login_time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `logout_time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `ip_addr4` INT UNSIGNED,
  `ip_addr6` BINARY(16),
  FOREIGN KEY (`UID`) REFERENCES `users`(`UID`)
);

CREATE TABLE `ships` (
  `id` INTEGER PRIMARY KEY NOT NULL,
  `Name` VARCHAR(50) NOT NULL,
  `Description` TEXT NOT NULL ,
  `Type` CHAR(1) NOT NULL
);

CREATE TABLE `teams` (
  `id` INTEGER PRIMARY KEY NOT NULL,
  `TeamName` VARCHAR(32) NOT NULL,
  `Member1` INTEGER NOT NULL,
  `Member2` INTEGER NOT NULL,
  `Member3` INTEGER NOT NULL,
  FOREIGN KEY (`Member1`) REFERENCES `users`(`UID`),
  FOREIGN KEY (`Member2`) REFERENCES `users`(`UID`),
  FOREIGN KEY (`Member3`) REFERENCES `users`(`UID`)
);

CREATE TABLE `game_history` (
  `id` INTEGER PRIMARY KEY NOT NULL,
  `Time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `Team1` INTEGER NOT NULL,
  `Team2` INTEGER NOT NULL,
  `Result` CHAR(1) NOT NULL,
  FOREIGN KEY (`Team1`) REFERENCES `teams`(`id`),
  FOREIGN KEY (`Team2`) REFERENCES `teams`(`id`)
);
