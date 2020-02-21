# jigglerBot
 A basic Twitch bot for speedrunners, designed in the spirit of the old WobberBot that was used for the Twitch team Wobblers.
 
 This is based off of an old branch that was given to a friend who happened to not speedrun, so conveniently the commands related to speedruns were removed. Since I wanted to make this open source, I plan to reimpliment the missing functions based off of the older code and work from there.
 
 **NOTE:** Due to the way the bot is designed, it does not need to be moderated in the chat it's being used in to function. Take extra caution not to use this in someone else's chat without their permission, because you might get banned (plus that's just not cool!)
 
 ## Features
 * Built in functions such as quoting, message of the day, and adding/removing of basic text commands
 * Ability to use your own account or a custom account using OAuth
 * Custom moderators list independent of Twitch chat moderators
 * Viewing and logging of Twitch chat via console
 
 ## Configuration
 Release versions will include a premade config.txt to store configuration options. If you are building from source, or if this file is removed, a new one will be generated on next start.
 
 ### Configuration Options
`username` - This is the bot's username. Can be your own, or one you create for it, but you must have access to it.

`oauthkey` - This is the OAuth key needed for the bot to access the account. Read more here: https://dev.twitch.tv/docs/authentication/getting-tokens-oauth

`channel` - The channel name that the bot should connect to on startup.

`motd` - Message of the day displayed by *!motd*. If autoMOTD is on, it will also be posted to the chat every 20 minutes.

`startMuted` - Whether to start muted or not. Useful for chat monitoring/logging without bot functions. 0 - not muted, 1 - muted

`autoMOTD` - Whether or not to post the message of the day automatically, once every 20 minutes. 0 - autoMOTD off, 1 - autoMOTD on

`cooldown` - Time in milliseconds to wait between queued command responses. 1500 is the minimum if not modded. If modded, the minimum is 300. **Going below the miniumum in either case will lock out the bot for 30 minutes.**

## Built In Commands
The bot comes with a selection of basic commands built in. Commands are executed from Twitch chat, **not** via the chat monitoring console. **Currently, you must add the first name to mods.txt in order to use admin commands.** Once added, you can add moderators using *!addmod.*

### Usable By Anyone

`!quote number` - Sends the specified quote to the chat.

`Custom Commands` - Commands added by *!addcom* are usable by anyone in chat.

### Admin/Moderator Commands
`!test` - Sends a test message to the chat to verify the bot is configured properly.

`!quit` or `!exit` - Shuts down the bot.

`!motd` - Sends the message of the day specified in config.txt

`!mutebot` - Toggles mute for the bot. While muted, it will not process or respond to commands and it will not post the MoTD.

`!addmod username` - Adds the specified username as a bot moderator. This allows them access to admin commands.

`!delmod username` - Removes the specified user from the bot moderator list.

`!addcom command output` - Adds a custom command that will send *output* to the chat when *!command* is used. Do not include the exclamation mark for *command*.

`!delcom command` - Removes a custom command.

`!reload` - Reloads the configuration and custom command list.

`!quote add quote` - Adds a quote to the quote list. The bot will reply with the quote's number.

`!quote remove number` - Removes the specified quote number from the quote list.

 ## Planned Features
 * SRC connectivity to list a game's current world record, user personal bests, etc
 * Integrating JigglerBot as the default account for responses, instead of requiring your own account
 * Disallowing cooldowns that would get the bot locked out, whether a moderator or not
 
 ~~* Random quoting when no number specified~~ Done!
 
 * Ability to turn off chat logging
 * Ability to recognize chat moderators/streamers
 
 ## Building
 The following libraries are needed to compile from source:
 * libircclient v1.8
 * curl v7.68
 
 It is currently Windows only.
