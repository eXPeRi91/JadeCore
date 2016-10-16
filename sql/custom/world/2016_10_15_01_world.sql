-- Fix Build
-- By Bodeguero for JadeCore.

UPDATE `creature_template` SET `difficulty_entry_1`=0 WHERE `difficulty_entry_1` IN (88885, 714760, 714790, 733490, 731911, 655221, 664131, 717120, 566371, 566841, 567311, 568631, 568671, 569241, 590751, 594641, 594791, 595181, 595191, 595201, 595221, 595511, 596051, 596841, 597041, 598221);
UPDATE `creature_template` SET `difficulty_entry_2`=0 WHERE `difficulty_entry_2`=684760;
UPDATE `creature_template` SET `difficulty_entry_4`=0 WHERE `difficulty_entry_4` IN (77007, 77028, 77029, 77032, 77006, 77021, 77001, 83983, 87100, 81621, 77009, 77010, 77020, 77023, 77013, 77003, 77014, 77015, 77016, 77024, 77025, 77026, 77002, 85393, 77027);
UPDATE `creature_template` SET `difficulty_entry_5`=0 WHERE `difficulty_entry_5` IN (77042, 77045, 77044, 77066, 77069, 77082, 77083);
UPDATE `creature_template` SET `difficulty_entry_6`=0 WHERE `difficulty_entry_6` IN (77104, 77107);
DELETE FROM `disables` WHERE `entry`=1134 AND `sourceType`=3;
DELETE FROM `disables` WHERE `entry` IN (126700, 138301);