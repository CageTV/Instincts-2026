ScriptName Mumirnik_Quest_UseFurniture Extends Quest
{ Exposes methods to force the player to use created furniture and delete it when they stand up.
  Rebuilt from the decompiled Instincts 2026-11-10 with one fix: the original deleted the seat when the player had not reached the
  sitting state within NumberOfTries x CheckUsageRate (6 s). In first person the sit takes longer, the seat vanished under a player the
  engine was still seating (sit state 2, "waiting for the sit animation"), and the furniture camera stayed on with no way out. Now the
  seat is kept while the engine still has the player in any sit state, and the camera is put back if the furniture camera stays on. }

;-- Variables ---------------------------------------
ObjectReference FurnitureREF
Int SavedCameraState = 0
Int WatchTicks = 0

;-- Properties --------------------------------------
Float Property CheckUsageRate Auto
Float Property HeightOffset Auto
Int Property NumberOfTries Auto
Actor Property PlayerREF Auto
; 2026-11-10: on this list the sit often never completes (first and, later, third person too: sit state 2 "waiting for the sit animation"
; forever, furniture camera stuck) and the training dialogue (TIF__05014C71) trains only when UseFurniture returns 0 or more. Training itself
; (Mumirnik_Quest_Instincts_PetTraining.Train) is only message boxes: the seat is cosmetic. With SkipSitting the seat is not used at all.
Bool Property SkipSitting = True Auto

;-- Functions ---------------------------------------

Int Function UseFurniture(Furniture akFurniture)
{ Creates a furniture at the player's feet and uses it. The furniture is deleted when the player stands up. Returns 1 if successful, 0 if player is sitting on something else, -1 if failed. }
  If SkipSitting
    Debug.Trace("[InstinctsSitFix] SkipSitting: training without the seat")
    Return 1
  EndIf
  Int sitState = PlayerREF.GetSitState()
  If sitState == 0
    SavedCameraState = Game.GetCameraState()
    If SavedCameraState == 0
      ; 2026-11-10: in first person the sit never completed on this list (third person does), so sit down from third person
      ; and go back to first person when the player has stood up (RemoveSeat)
      Game.ForceThirdPerson()
      Utility.Wait(0.5)
    EndIf
    FurnitureREF = PlayerREF.PlaceAtMe(akFurniture as Form, 1, False, True)
    Float zAngle = PlayerREF.GetAngleZ()
    If HeightOffset != 0 as Float
      FurnitureREF.MoveTo(PlayerREF as ObjectReference, 0.0, 0.0, HeightOffset, True)
    EndIf
    FurnitureREF.SetAngle(0.0, 0.0, zAngle)
    FurnitureREF.Enable(False)
    FurnitureREF.Activate(PlayerREF as ObjectReference, True)
    Int i = 0
    While PlayerREF.GetSitState() != 3 && i < NumberOfTries
      Utility.Wait(CheckUsageRate)
      i += 1
    EndWhile
    Int finalState = PlayerREF.GetSitState()
    If finalState != 3
      Debug.Trace("[InstinctsSitFix] not sitting after " + i + " tries: sit state " + finalState + ", camera state " + Game.GetCameraState())
      If finalState != 0
        ; the engine is still seating the player: the seat stays; OnUpdate removes it once the sit state is back to 0
        WatchTicks = 0
        Self.RegisterForSingleUpdate(CheckUsageRate)
      Else
        RemoveSeat(False)
      EndIf
      Return -1
    Else
      Debug.Trace("[InstinctsSitFix] sitting after " + i + " tries")
      WatchTicks = 0
      Self.RegisterForSingleUpdate(CheckUsageRate)
      Return 1
    EndIf
  ElseIf sitState == 3
    Return 0
  Else
    Return -1
  EndIf
EndFunction

Event OnUpdate()
  If PlayerREF.GetSitState() != 0
    WatchTicks += 1
    If WatchTicks == 240
      Debug.Trace("[InstinctsSitFix] still in sit state " + PlayerREF.GetSitState() + " after about 2 minutes, camera state " + Game.GetCameraState())
    EndIf
    Self.RegisterForSingleUpdate(CheckUsageRate)
  Else
    Utility.Wait(1.0)
    RemoveSeat(True)
  EndIf
EndEvent

Function RemoveSeat(Bool abCheckCamera)
  If FurnitureREF
    FurnitureREF.Disable(True)
    FurnitureREF.Delete()
    FurnitureREF = None
  EndIf
  If SavedCameraState == 0
    ; the sit was started from third person (UseFurniture); the player was in first person before it
    Debug.Trace("[InstinctsSitFix] back to first person")
    Game.ForceFirstPerson()
  ElseIf abCheckCamera
    Int cameraState = Game.GetCameraState()
    If cameraState == 5
      ; the furniture camera is still on although the player is not seated: put the camera back where it was
      Debug.Trace("[InstinctsSitFix] furniture camera still on after the sit: restoring third person")
      Game.ForceThirdPerson()
    EndIf
  EndIf
EndFunction
