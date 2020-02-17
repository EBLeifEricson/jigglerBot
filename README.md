# jigglerBot
 A basic Twitch bot for speedrunners.
 
 This is based off of an old branch that was given to a friend who happened to not speedrun, so conveniently the commands related to speedruns were removed. Since I wanted to make this open source, I plan to reimpliment the missing functions based off of the older code and work from there.
 
 **NOTE:** Due to the way the bot is designed, it does not need to be moderated in the chat it's being used in to function. Take extra caution not to use this in someone else's chat without their permission, because you might get banned (plus that's just not cool!)
 
 Features
 * Built in functions such as quoting, message of the day, and adding/removing of basic text commands
 * Ability to use your own account or a custom account using OAuth
 * Custom moderators list independent of Twitch chat moderators
 * Viewing and logging of Twitch chat via console
 
 Planned Features
 * SRC connectivity to list a game's current world record, user personal bests, etc
 
 The following libraries are needed to compile from source:
 * libircclient v1.8
 * curl v7.68
 
 It is currently Windows only.
