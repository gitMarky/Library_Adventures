

public func OnBroadcastDialogue(string message, object player, object speaker)
{
	MessageBox(message, player, speaker);
}


public func OnBroadcastClose(object player)
{
	CloseMessageBox(player);
}


public func OnBroadcastDestruction()
{
	RemoveObject();
}

//------------------------------------------------------------------------
// message boxes
// since these are unchanged... maybe have them in another library in the objects??

local dlg_broadcast; // if set, all non-message (i.e. menu) MessageBox calls are called as MessageBoxBroadcast.

private func CloseMessageBox(object player)
{
	player->CloseMenu();
}


/**
 A message box for all players 
 @author Sven2
 @version 0.1.0
 */
public func MessageBoxAll(string message, object speaker, bool as_message)
{
	for(var i = 0; i < GetPlayerCount(C4PT_User); ++i)
	{
		var plr = GetPlayerByIndex(i, C4PT_User);
		MessageBox(message, GetCursor(plr), speaker, plr, as_message);
	}
}


/**
 Message box as dialog to player with a message copy to all other players
 @author Sven2
 @version 0.1.0
 */
public func MessageBoxBroadcast(string message, object player, object speaker, array options)
{
	// message copy to other players
	for(var i = 0; i < GetPlayerCount(C4PT_User); ++i)
	{
		var plr = GetPlayerByIndex(i, C4PT_User);
		if (GetCursor(plr) != player)
			MessageBox(message, GetCursor(plr), speaker, plr, true);
	}
	// main message as dialog box
	return MessageBox(message, player, speaker, nil, false, options);
}

static MessageBox_last_speaker, MessageBox_last_pos;


/**
 A single message box.
 @author Sven2
 @version 0.1.0
 */
private func MessageBox(string message, object player, object speaker, int for_player, bool as_message, array options)
{
	// broadcast enabled: message copy to other players
	if (dlg_broadcast && !as_message)
	{
		for(var i = 0; i < GetPlayerCount(C4PT_User); ++i)
		{
			var other_plr = GetPlayerByIndex(i, C4PT_User);
			if (GetCursor(other_plr) != player)
				MessageBox(message, GetCursor(other_plr), speaker, other_plr, true);
		}
	}
	// Use current NPC as speaker if unspecified.
	// On definition call or without speaker, just show the message without a source
	var portrait;
	if (!speaker && this != DialogueEx) speaker = this; //dlg_target;
	if (speaker)
	{
		message = Format("<c %x>%s:</c> %s", speaker->GetColor(), speaker->GetName(), message);
	    portrait = speaker->~GetPortrait();
	}
	
	// A target player is given: Use a menu for this dialogue.
	if (player && !as_message)
	{
		var menu_target, cmd;
		if (this != DialogueEx)
		{
			menu_target = this;
			cmd = "MenuOK";
		}
		player->CreateMenu(DialogueEx, menu_target, C4MN_Extra_None, nil, nil, C4MN_Style_Dialog, false, DialogueEx);
		
		// Add NPC portrait.
		//var portrait = Format("%i", speaker->GetID()); //, DialogueEx, speaker->GetColor(), "1");
		if (speaker)
			if (portrait)
				player->AddMenuItem("", nil, DialogueEx, nil, player, nil, C4MN_Add_ImgPropListSpec, portrait);
			else
				player->AddMenuItem("", nil, DialogueEx, nil, player, nil, C4MN_Add_ImgObject, speaker);

		// Add NPC message.
		player->AddMenuItem(message, nil, nil, nil, player, nil, C4MN_Add_ForceNoDesc);
		
		// Add answers.
		if (options) for (var option in options)
		{
			var option_text, option_command;
			if (GetType(option) == C4V_Array)
			{
				// Text+Command given
				option_text = option[0];
				option_command = option[1];
				if (GetChar(option_command) == GetChar("#"))
				{
					// Command given as section name: Remove leading # and call section change
					var ichar=1, ocmd = "", c;
					while (c = GetChar(option_command, ichar++)) ocmd = Format("%s%c", ocmd, c);
					option_command = Format("CallDialogue(Object(%d), 1, \"%s\")", player->ObjectNumber(), ocmd);
				}
				else
				{
					// if only a command is given, the standard parameter is just the player
					if (!WildcardMatch(option_command, "*(*")) option_command = Format("%s(Object(%d))", option_command, player->ObjectNumber());
				}
			}
			else
			{
				// Only text given - command means regular dialogue advance
				option_text = option;
				option_command = cmd;
			}
			player->AddMenuItem(option_text, option_command, nil, nil, player, nil, C4MN_Add_ForceNoDesc);
		}
		
		// If there are no answers, add a next entry
		if (cmd && !options) player->AddMenuItem("$Next$", cmd, nil, nil, player, nil, C4MN_Add_ForceNoDesc);
		
		// Set menu decoration.
		player->SetMenuDecoration(GUI_MenuDeco);
		
		// Set text progress to NPC name.
		if (speaker)
		{
			var name = speaker->GetName();
			var n_length;
			while (GetChar(name, n_length))
				n_length++;
			player->SetMenuTextProgress(n_length + 1);
		}
	}
	else
	{
		// No target is given: Global (player) message
		if (!GetType(for_player)) for_player = NO_OWNER;
		// altenate left/right position as speakers change
		if (speaker != MessageBox_last_speaker) MessageBox_last_pos = !MessageBox_last_pos;
		MessageBox_last_speaker = speaker;
		var flags = 0, xoff = 150;
		if (!MessageBox_last_pos)
		{
			flags = MSG_Right;
			xoff *= -1;
			CustomMessage("", nil, for_player); // clear prev msg
		}
		else
		{
			CustomMessage("", nil, for_player, 0, 0, nil, nil, nil, MSG_Right);  // clear prev msg
		}
		CustomMessage(message, nil, for_player, xoff, 150, nil, GUI_MenuDeco, portrait ?? speaker, flags);
	}

	return;
}