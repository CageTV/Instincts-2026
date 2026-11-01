ScriptName Mumirnik_Quest_UseFurniture Extends Quest
{ Exposes methods to force the player to use created furniture and delete it when they stand up. }

;-- Variables ---------------------------------------
ObjectReference FurnitureREF

;-- Properties --------------------------------------
Float Property CheckUsageRate Auto
Float Property HeightOffset Auto
Int Property NumberOfTries Auto
Actor Property PlayerREF Auto

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Int Function UseFurniture(Furniture akFurniture)
{ Creates a furniture at the player's feet and uses it. The furniture is deleted when the player stands up. Returns 1 if successful, 0 if player is sitting on something else, -1 if failed. }
  Int sitState = PlayerREF.GetSitState() ; #DEBUG_LINE_NO:15
  If sitState == 0 ; #DEBUG_LINE_NO:16
    FurnitureREF = PlayerREF.PlaceAtMe(akFurniture as Form, 1, False, True) ; #DEBUG_LINE_NO:17
    Float zAngle = PlayerREF.GetAngleZ() ; #DEBUG_LINE_NO:18
    If HeightOffset != 0 as Float ; #DEBUG_LINE_NO:19
      FurnitureREF.MoveTo(PlayerREF as ObjectReference, 0.0, 0.0, HeightOffset, True) ; #DEBUG_LINE_NO:20
    EndIf
    FurnitureREF.SetAngle(0.0, 0.0, zAngle) ; #DEBUG_LINE_NO:22
    FurnitureREF.Enable(False) ; #DEBUG_LINE_NO:23
    FurnitureREF.Activate(PlayerREF as ObjectReference, True) ; #DEBUG_LINE_NO:24
    Int i = 0 ; #DEBUG_LINE_NO:26
    While PlayerREF.GetSitState() != 3 && i < NumberOfTries ; #DEBUG_LINE_NO:27
      Utility.Wait(CheckUsageRate) ; #DEBUG_LINE_NO:28
      i += 1 ; #DEBUG_LINE_NO:29
    EndWhile
    If i >= NumberOfTries ; #DEBUG_LINE_NO:32
      If FurnitureREF ; #DEBUG_LINE_NO:33
        FurnitureREF.Disable(True) ; #DEBUG_LINE_NO:34
        FurnitureREF.Delete() ; #DEBUG_LINE_NO:35
      EndIf
      Return -1 ; #DEBUG_LINE_NO:37
    Else
      Self.RegisterForSingleUpdate(CheckUsageRate) ; #DEBUG_LINE_NO:39
      Return 1 ; #DEBUG_LINE_NO:40
    EndIf
  ElseIf sitState == 3 ; #DEBUG_LINE_NO:42
    Return 0 ; #DEBUG_LINE_NO:43
  Else
    Return -1 ; #DEBUG_LINE_NO:45
  EndIf
EndFunction

Event OnUpdate()
  If PlayerREF.GetSitState() != 0 ; #DEBUG_LINE_NO:50
    Self.RegisterForSingleUpdate(CheckUsageRate) ; #DEBUG_LINE_NO:51
  ElseIf FurnitureREF
    FurnitureREF.Disable(True) ; #DEBUG_LINE_NO:54
    FurnitureREF.Delete() ; #DEBUG_LINE_NO:55
  EndIf
EndEvent
