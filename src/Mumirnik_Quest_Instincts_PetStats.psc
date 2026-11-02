ScriptName Mumirnik_Quest_Instincts_PetStats Extends Quest
{ Manages all pet stats. Exposes functions to modify innate pet stats, modify stats, and display stats. }

;-- Variables ---------------------------------------
Bool StarvationWarningTriggered = False

;-- Properties --------------------------------------
String Property ExperienceFortifyAVName Auto
{ The name of the actor value for Fortify Experience effects. }
String Property HungerAVName Auto
{ The name of the actor value that tracks Hunger. }
Message[] Property HungerChangeMessage Auto
GlobalVariable Property HungerDecayRateGlobal Auto
GlobalVariable Property HungerDecayRateWhenWaitingGlobal Auto
String Property HungerFortifyAVName Auto
{ The name of the actor value for Fortify Hunger effects. }
Message[] Property HungerLimitMaxMessage Auto
Message[] Property HungerLimitMinMessage Auto
Keyword Property IsUnaggressivePetRace Auto
String Property MountedSpeedAVName Auto
{ The name of the actor value for Mounted Speed effects. }
GlobalVariable Property PetCount Auto
Message[] Property PetStatisticsMessageF Auto
Message[] Property PetStatisticsMessageM Auto
Float Property AutoEatBelow = 50.0 Auto
{ A pet at or below this Hunger (0-100) eats food from its own inventory once per game hour. 0 = never. }
Int Property AutoEatMaxMeals = 3 Auto
{ Most food items a pet eats in one go. }

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Function ShowPetStats(Actor akTarget, Int aiSlot)
{ Shows a message box with the pet's current stats and level. }
  If aiSlot == -1 ; #DEBUG_LINE_NO:26
    aiSlot = ((Self as Quest) as mumirnik_quest_instincts_petoptions).GetSlotNumberForActor(akTarget) ; #DEBUG_LINE_NO:27
  EndIf
  String petLevelAVName = ((Self as Quest) as mumirnik_quest_instincts_pettraining).petLevelAVName ; #DEBUG_LINE_NO:29
  String petLevelProgressAVName = ((Self as Quest) as mumirnik_quest_instincts_pettraining).petLevelProgressAVName ; #DEBUG_LINE_NO:30
  String petGenderAVName = ((Self as Quest) as mumirnik_quest_instincts_petoptions).GenderAVName ; #DEBUG_LINE_NO:31
  String petPowerAVName = ((Self as Quest) as mumirnik_quest_instincts_petoptions).PowerAVName ; #DEBUG_LINE_NO:32
  Float petLevel = akTarget.GetActorValue(petLevelAVName) ; #DEBUG_LINE_NO:33
  Float petLevelProgress = akTarget.GetActorValue(petLevelProgressAVName) ; #DEBUG_LINE_NO:34
  Float petDamage = akTarget.GetActorValue("UnarmedDamage") ; #DEBUG_LINE_NO:35
  Float petCarryWeight = akTarget.GetActorValue("CarryWeight") ; #DEBUG_LINE_NO:36
  Float petHealthMax = akTarget.GetActorValue("Health") ; #DEBUG_LINE_NO:37
  Float petHungerModifier = akTarget.GetActorValue(HungerFortifyAVName) ; #DEBUG_LINE_NO:38
  Float petExperienceModifier = akTarget.GetActorValue(ExperienceFortifyAVName) ; #DEBUG_LINE_NO:39
  Float petPower = akTarget.GetActorValue(petPowerAVName) ; #DEBUG_LINE_NO:40
  If akTarget.GetActorValue(petGenderAVName) == 2 as Float ; #DEBUG_LINE_NO:41
    PetStatisticsMessageF[aiSlot].Show(petLevel, (petLevelProgress as Int) as Float, (petLevelProgress - (petLevelProgress as Int) as Float) * 100 as Float, petDamage, petCarryWeight, petHealthMax, petHungerModifier, petExperienceModifier, petPower) ; #DEBUG_LINE_NO:42
  Else
    PetStatisticsMessageM[aiSlot].Show(petLevel, (petLevelProgress as Int) as Float, (petLevelProgress - (petLevelProgress as Int) as Float) * 100 as Float, petDamage, petCarryWeight, petHealthMax, petHungerModifier, petExperienceModifier, petPower) ; #DEBUG_LINE_NO:44
  EndIf
EndFunction

Function UpdatePetStatsGameTime(Actor akTarget)
{ Ticks every hour on each pet. Handles hunger decay and experience growth. }
  Bool isWaiting = akTarget.GetActorValue("WaitingForPlayer") > 0.0 ; #DEBUG_LINE_NO:50
  Float hungerDecay = 0.0 ; #DEBUG_LINE_NO:51
  If isWaiting ; #DEBUG_LINE_NO:52
    hungerDecay = HungerDecayRateWhenWaitingGlobal.GetValue() ; #DEBUG_LINE_NO:53
  Else
    hungerDecay = HungerDecayRateGlobal.GetValue() ; #DEBUG_LINE_NO:55
  EndIf
  hungerDecay *= (100.0 - akTarget.GetActorValue(HungerFortifyAVName)) / 100.0 ; #DEBUG_LINE_NO:57
  If hungerDecay < 0 as Float ; #DEBUG_LINE_NO:58
    hungerDecay = 0 as Float ; #DEBUG_LINE_NO:59
  EndIf
  Self.ModHunger(akTarget, -hungerDecay) ; #DEBUG_LINE_NO:61
  Self.AutoEat(akTarget)
  String petLevelAVName = ((Self as Quest) as mumirnik_quest_instincts_pettraining).petLevelAVName ; #DEBUG_LINE_NO:63
  Int petLevel = akTarget.GetActorValue(petLevelAVName) as Int ; #DEBUG_LINE_NO:64
  Float progressAmount = 1.0 / (petLevel as Float + 1.0) ; #DEBUG_LINE_NO:65
  ((Self as Quest) as mumirnik_quest_instincts_pettraining).Progress(akTarget, progressAmount) ; #DEBUG_LINE_NO:72
EndFunction

Function AutoEat(Actor akTarget)
{ A hungry pet eats food it carries. Uses the same species food list as the feeding menu. }
  If AutoEatBelow <= 0.0 || akTarget == None || akTarget.IsDead() || akTarget.IsInCombat()
    Return
  EndIf
  Quest q = Self as Quest
  Int raceId = (q as mumirnik_quest_instincts_petoptions).NewRaceList.Find(akTarget.GetRace() as Form)
  If raceId < 0
    Return
  EndIf
  GlobalVariable foodType = (q as mumirnik_quest_instincts_feeding).FoodTypes.GetAt(raceId) as GlobalVariable
  If foodType == None
    Return
  EndIf
  FormList foodList = (q as mumirnik_quest_instincts_feeding).FoodSublists.GetAt(foodType.GetValue() as Int) as FormList
  If foodList == None
    Return
  EndIf
  Int meals = 0
  While meals < AutoEatMaxMeals && akTarget.GetActorValue(HungerAVName) <= AutoEatBelow
    Form food = None
    Int i = akTarget.GetNumItems()
    While i > 0 && food == None
      i -= 1
      Form item = akTarget.GetNthForm(i)
      If item != None && foodList.HasForm(item)
        food = item
      EndIf
    EndWhile
    If food == None
      Return
    EndIf
    (q as mumirnik_quest_instincts_feeding).ReceiveFood(akTarget, food, 1)
    meals += 1
  EndWhile
EndFunction

Function SetHunger(Actor akTarget, Float aiValue)
{ Updates the hunger stat and refreshes the visual display. Call this to change hunger. }
  akTarget.SetActorValue(HungerAVName, aiValue) ; #DEBUG_LINE_NO:77
  ((Self as Quest) as mumirnik_quest_instincts_petoptions).UpdateHungerAbilities(akTarget) ; #DEBUG_LINE_NO:79
  ((Self as Quest) as mumirnik_quest_instincts_petoptions).RefreshHungerDisplay(akTarget) ; #DEBUG_LINE_NO:80
EndFunction

Function ModHunger(Actor akTarget, Float aiValue)
{ Updates the hunger stat and refreshes the visual display. Call this to change hunger. }
  akTarget.ModActorValue(HungerAVName, aiValue) ; #DEBUG_LINE_NO:85
  Float value = akTarget.GetActorValue(HungerAVName) ; #DEBUG_LINE_NO:87
  If value < 0 as Float ; #DEBUG_LINE_NO:88
    akTarget.ModActorValue(HungerAVName, -value) ; #DEBUG_LINE_NO:89
    If (akTarget.GetActorValue("WaitingForPlayer") > 0 as Float) && !StarvationWarningTriggered ; #DEBUG_LINE_NO:90
      Int slotNumber = ((Self as Quest) as mumirnik_quest_instincts_petoptions).GetSlotNumberForActor(akTarget) ; #DEBUG_LINE_NO:91
      HungerLimitMinMessage[slotNumber].Show(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0) ; #DEBUG_LINE_NO:92
      StarvationWarningTriggered = True ; #DEBUG_LINE_NO:93
    EndIf
  Else
    StarvationWarningTriggered = False ; #DEBUG_LINE_NO:96
    If value > 100 as Float ; #DEBUG_LINE_NO:97
      akTarget.ModActorValue(HungerAVName, 100 as Float - value) ; #DEBUG_LINE_NO:98
      Int slotnumber = ((Self as Quest) as mumirnik_quest_instincts_petoptions).GetSlotNumberForActor(akTarget) ; #DEBUG_LINE_NO:99
      HungerLimitMaxMessage[slotnumber].Show(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0) ; #DEBUG_LINE_NO:100
    EndIf
  EndIf
  ((Self as Quest) as mumirnik_quest_instincts_petoptions).UpdateHungerAbilities(akTarget) ; #DEBUG_LINE_NO:104
  ((Self as Quest) as mumirnik_quest_instincts_petoptions).RefreshHungerDisplay(akTarget) ; #DEBUG_LINE_NO:105
EndFunction
