ScriptName Mumirnik_Quest_Instincts_PetOptions Extends Quest
{ The guts of the pet system. Handles pet taming and releasing, and manages pet slots and the slot cap. }

;-- Variables ---------------------------------------
Actor AnimalToTame
Float TameChance = 0.0
Actor RememberedAnimal
Float RememberedAt = 0.0
Bool TimerIsRunning = False

;-- Properties --------------------------------------
Message Property ChanceToTame Auto
Message Property ChanceToTameFAILED Auto
Race[] Property ForcedFemaleRaceList Auto
{ Original races in this list will always become female. }
Race[] Property ForcedMaleRaceList Auto
{ Original races in this list will always become male. }
Race[] Property ForcedRandomGenderMaleVisualsRaceList Auto
{ Original races in this list will become a random gender, but will use male visuals. }
Race[] Property ForcedRandomGenderRaceList Auto
{ Original races in this list will become a random gender and get the appropriate visuals. }
String Property GenderAVName Auto
{ M = 1, F = 2 }
Spell[] Property HungerBuffSpell Auto
{ MUST be at least 6 items. Hunger progression is hardcoded. }
Keyword Property IsUnaggressivePetRaceKeyword Auto
FormList Property NewRaceList Auto
FormList Property OriginalRaceList Auto
{ List of races that can be turned into pets. }
FormList Property PetActorListF Auto
{ List of pet actors. }
FormList Property PetActorListM Auto
{ List of pet actors. }
GlobalVariable Property PetCount Auto
{ Current number of pets. }
Message Property PetCountCheckFAILED Auto
Message Property PetEssentialCheckFAILED Auto
Activator[] Property PetHungerActivator Auto
{ MUST be at least 6 items. Hunger progression is hardcoded. }
ReferenceAlias[] Property PetHungerREF Auto
Activator Property PetNameBaseActivator Auto
FormList Property PetNameF Auto
FormList Property PetNameM Auto
ReferenceAlias[] Property PetNameREF Auto
FormList Property PetNameUnaggressiveF Auto
FormList Property PetNameUnaggressiveM Auto
GlobalVariable Property PetPower Auto
{ Total amount of power among pets. Used to prevent stacking powerful pets. }
Message Property PetPowerCheckFAILED Auto
GlobalVariable Property PetPowerMax Auto
FormList Property PetPowerValuesList Auto
{ The power for each pet race/original race. }
ReferenceAlias[] Property PetREF Auto
Actor Property PlayerREF Auto
String Property PowerAVName Auto
{ Set on the pet to help with release and stats. }
Perk Property PowerBudgetPerk1 Auto
Perk Property PowerBudgetPerk2 Auto
Perk Property PowerBudgetPerk3 Auto
Perk Property PowerMaxPerk Auto
ActorBase[] Property RaceSubstituteActorListF Auto
{ Corresponding race substitutes for actorbases that will be special cased. }
ActorBase[] Property RaceSubstituteActorListM Auto
{ Corresponding race substitutes for actorbases that will be special cased. }
ActorBase[] Property RaceSubstituteOriginalActorBaseList Auto
{ Actorbases that will be special cased. If the actor is one of these actorbases, it will take the pet actor from the RaceSubstitute lists instead of using its actual race. }
GlobalVariable Property SpeechcraftExperienceGainTaming Auto
Message[] Property TamedMessage Auto
{ Shown when a pet is tamed. }
Sound Property TamedSound Auto
Bool Property UsePerkBasedPowerBudget = False Auto

Float Property RememberSeconds = 120.0 Auto
{ A failed taming attempt keeps its accumulated chance for this many real seconds (counted from the last feeding) for the same animal. }
Int Property SlotLimit = 3 Auto
{ Total pet slots (a big animal takes more than one). Was a hard-coded 3. }
Int Property BigPower = 6 Auto
{ An animal of this power or more takes 2 slots. }
Int Property HugePower = 10 Auto
{ An animal of this power or more takes 3 slots. }
Int Property LevelBig = 20 Auto
{ Player level needed to tame an animal of BigPower (6) or more. }
Int Property LevelGreat = 30 Auto
{ Player level needed to tame an animal of power 8 or more. }
Int Property LevelHuge = 40 Auto
{ Player level needed to tame an animal of HugePower (10) or more. }

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Bool Function CheckPowerBudget(Actor akTarget)
{ Returns whether the target can be made into a pet without exceeding the power budget. True if yes, false and error message if no. }
  Int powerMax = 0 ; #DEBUG_LINE_NO:80
  If UsePerkBasedPowerBudget ; #DEBUG_LINE_NO:81
    If PlayerREF.HasPerk(PowerBudgetPerk3) ; #DEBUG_LINE_NO:82
      powerMax = 10 ; #DEBUG_LINE_NO:83
    ElseIf PlayerREF.HasPerk(PowerBudgetPerk2) ; #DEBUG_LINE_NO:84
      powerMax = 8 ; #DEBUG_LINE_NO:85
    ElseIf PlayerREF.HasPerk(PowerBudgetPerk1) ; #DEBUG_LINE_NO:86
      powerMax = 6 ; #DEBUG_LINE_NO:87
    Else
      powerMax = 4 ; #DEBUG_LINE_NO:89
    EndIf
  Else
    Float playerSkill = PlayerREF.GetActorValue("Speechcraft") ; #DEBUG_LINE_NO:92
    powerMax = 0 ; #DEBUG_LINE_NO:93
    If playerSkill < 20 as Float ; #DEBUG_LINE_NO:94
      powerMax = 6 ; #DEBUG_LINE_NO:95
    ElseIf playerSkill < 40 as Float ; #DEBUG_LINE_NO:96
      powerMax = 7 ; #DEBUG_LINE_NO:97
    ElseIf playerSkill < 60 as Float ; #DEBUG_LINE_NO:98
      powerMax = 8 ; #DEBUG_LINE_NO:99
    ElseIf playerSkill < 80 as Float ; #DEBUG_LINE_NO:100
      powerMax = 9 ; #DEBUG_LINE_NO:101
    Else
      powerMax = 10 ; #DEBUG_LINE_NO:103
    EndIf
    If PlayerREF.HasPerk(PowerMaxPerk) ; #DEBUG_LINE_NO:105
      powerMax += 1 ; #DEBUG_LINE_NO:106
    EndIf
  EndIf
  PetPowerMax.SetValue(powerMax as Float) ; #DEBUG_LINE_NO:109
  Race originalRace = akTarget.GetRace()
  Int originalRaceId = OriginalRaceList.Find(originalRace as Form)
  If originalRaceId < 0
    Debug.Notification("This animal cannot be tamed.")
    Return False
  EndIf
  Int power = Self.GetPowerForRaceId(originalRaceId)
  Int currentPetPower = PetPower.GetValue() as Int ; #DEBUG_LINE_NO:118
  Int currentPetPowerMax = PetPowerMax.GetValue() as Int ; #DEBUG_LINE_NO:119
  If currentPetPower + power > currentPetPowerMax ; #DEBUG_LINE_NO:120
    PetPowerCheckFAILED.Show(currentPetPower as Float, currentPetPowerMax as Float, power as Float, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0) ; #DEBUG_LINE_NO:121
    Return False ; #DEBUG_LINE_NO:122
  EndIf
  Int needLevel = Self.LevelNeededForPower(power)
  If PlayerREF.GetLevel() < needLevel
    Debug.Notification("You are not experienced enough to tame this animal (level " + needLevel + ").")
    Return False
  EndIf
  Int cost = Self.SlotCostForPower(power)
  Self.SyncPetCount()
  If (PetCount.GetValue() as Int) + cost > Self.GetSlotLimit()
    PetCountCheckFAILED.Show(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0)
    If cost > 1
      Debug.Notification("This animal needs " + cost + " pet slots (" + (Self.GetSlotLimit() - (PetCount.GetValue() as Int)) + " free).")
    EndIf
    Return False
  EndIf
  Return True
EndFunction

Function TryMakePet(Actor akTarget, Float aiValue)
{ This is called from the taming feature. It is a thread safe way to make a target your pet. }
  Float now = Utility.GetCurrentRealTime()
  If akTarget != RememberedAnimal || now < RememberedAt || now - RememberedAt > RememberSeconds
    TameChance = 0.0 ; another animal, a long pause, or a new session: start from nothing
  EndIf
  RememberedAnimal = akTarget
  RememberedAt = now
  AnimalToTame = akTarget ; #DEBUG_LINE_NO:135
  TameChance += aiValue ; #DEBUG_LINE_NO:136
  ChanceToTame.Show(TameChance, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0) ; #DEBUG_LINE_NO:137
  Self.RegisterForSingleUpdate(0.02) ; #DEBUG_LINE_NO:138
EndFunction

Event OnUpdate()
  If AnimalToTame ; #DEBUG_LINE_NO:142
    If Utility.RandomInt(0, 99) as Float < TameChance ; #DEBUG_LINE_NO:143
      Self.MakePet(AnimalToTame) ; #DEBUG_LINE_NO:144
      TameChance = 0.0
      RememberedAnimal = None
    Else
      ChanceToTameFAILED.Show(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0) ; #DEBUG_LINE_NO:146
    EndIf
    AnimalToTame = None ; #DEBUG_LINE_NO:148
  EndIf
EndEvent

Function MakePet(Actor akTarget)
{ Makes the target your pet. The wild animal is only removed AFTER the pet exists in a free slot: an animal that cannot be a pet is left alone. }
  If !akTarget || akTarget.IsDead()
    Return
  EndIf
  Race originalRace = akTarget.GetRace()
  Int originalRaceId = OriginalRaceList.Find(originalRace as Form)
  If originalRaceId < 0
    Debug.Notification("This animal cannot be tamed.")
    Debug.Trace("[Instincts+] MakePet refused: race not on the tamable list: " + originalRace)
    Return
  EndIf
  Bool isForcedRandomGenderMaleVisuals = ForcedRandomGenderMaleVisualsRaceList.find(originalRace, 0) != -1
  Int gender = 0
  If ForcedFemaleRaceList.find(originalRace, 0) != -1
    gender = 2
  ElseIf ForcedMaleRaceList.find(originalRace, 0) != -1
    gender = 1
  ElseIf ForcedRandomGenderRaceList.find(originalRace, 0) != -1 || isForcedRandomGenderMaleVisuals
    gender = Utility.RandomInt(1, 2)
  Else
    gender = akTarget.GetLeveledActorBase().GetSex() + 1
  EndIf
  ActorBase newActorBase = None
  If gender == 1 || isForcedRandomGenderMaleVisuals
    newActorBase = PetActorListM.GetAt(originalRaceId) as ActorBase
  ElseIf gender == 2
    newActorBase = PetActorListF.GetAt(originalRaceId) as ActorBase
  EndIf
  ActorBase targetActorBase = akTarget.GetLeveledActorBase()
  Int raceSubstituteActorBaseId = RaceSubstituteOriginalActorBaseList.find(targetActorBase, 0)
  If raceSubstituteActorBaseId == -1
    targetActorBase = akTarget.GetBaseObject() as ActorBase
    raceSubstituteActorBaseId = RaceSubstituteOriginalActorBaseList.find(targetActorBase, 0)
  EndIf
  Int targetActorBaseId = targetActorBase.GetFormID()
  If targetActorBaseId == 67348929 && originalRace.GetFormID() == 78346
    raceSubstituteActorBaseId = 9
  ElseIf targetActorBaseId == 271029 && originalRace.GetFormID() == 78346 && akTarget.GetLevel() >= 6
    raceSubstituteActorBaseId = 9
  ElseIf targetActorBaseId == 271024
    Int originalRaceFormID = originalRace.GetFormID()
    If originalRaceFormID == 78328
      raceSubstituteActorBaseId = 5
    ElseIf originalRaceFormID == 341111
      raceSubstituteActorBaseId = 3
    EndIf
  ElseIf targetActorBaseId == 124835 && originalRace.GetFormID() == 78346 && akTarget.GetLevel() >= 6
    raceSubstituteActorBaseId = 9
  ElseIf targetActorBaseId == 416315 && originalRace.GetFormID() == 78346 && akTarget.GetLevel() >= 6
    raceSubstituteActorBaseId = 9
  ElseIf targetActorBaseId == 969066 && originalRace.GetFormID() == 1088636
    raceSubstituteActorBaseId = 0
  EndIf
  If raceSubstituteActorBaseId != -1
    If gender == 1 || isForcedRandomGenderMaleVisuals
      newActorBase = RaceSubstituteActorListM[raceSubstituteActorBaseId]
      originalRaceId = PetActorListM.Find(newActorBase as Form)
    ElseIf gender == 2
      newActorBase = RaceSubstituteActorListF[raceSubstituteActorBaseId]
      originalRaceId = PetActorListF.Find(newActorBase as Form)
    EndIf
  EndIf
  If !newActorBase || originalRaceId < 0
    Debug.Notification("This animal cannot be tamed.")
    Debug.Trace("[Instincts+] MakePet refused: no pet actor for " + targetActorBase + " (race " + originalRace + ", id " + originalRaceId + ")")
    Return
  EndIf
  Int slot = -1
  Int s = 0
  While s < PetREF.Length && slot == -1
    If PetREF[s].GetReference() == None
      slot = s
    EndIf
    s += 1
  EndWhile
  If slot == -1
    Debug.Notification("No free pet slot.")
    Debug.Trace("[Instincts+] MakePet refused: no free slot")
    Return
  EndIf
  Actor petActor = akTarget.PlaceActorAtMe(newActorBase, 4, None)
  If !petActor
    Debug.Trace("[Instincts+] MakePet failed: could not place " + newActorBase)
    Return
  EndIf
  petActor.MoveTo(akTarget as ObjectReference, 0.0, 0.0, 0.0, True)
  PetREF[slot].ForceRefTo(petActor as ObjectReference)
  BeastHarmony_Look.CopyLook(akTarget, petActor) ; Beast Harmony: the pet wears the wild animal's variant skin
  ; the pet exists and holds a slot: now the wild animal can go
  akTarget.SetAlpha(0.0, True)
  TamedSound.Play(petActor as ObjectReference)
  ; Instincts+: the wild animal used to be Kill(PlayerREF)ed here, which made every OnDeath listener (Hunting in Skyrim hunt quests, kill counters) treat a taming as a player kill. It is removed without dying now.
  akTarget.Disable(True)
  akTarget.Delete()
  petActor.StopCombat()
  petActor.SetPlayerTeammate(True, False)
  petActor.EvaluatePackage()
  TamedMessage[slot].Show(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0)
  petActor.RestoreActorValue("Health", 10000 as Float)
  ((Self as Quest) as mumirnik_quest_instincts_petstats).SetHunger(petActor, 50 as Float)
  ((Self as Quest) as mumirnik_quest_instincts_pettraining).InitTrainingProgression(petActor, originalRaceId)
  Self.SetBaseNameDisplay(petActor, gender)
  Int power = Self.GetPowerForRaceId(originalRaceId)
  PetPower.Mod(power as Float)
  petActor.SetActorValue(PowerAVName, power as Float)
  Self.SyncPetCount()
  Debug.Trace("[Instincts+] tamed " + newActorBase + " in slot " + slot + " (power " + power + ", slots in use " + (PetCount.GetValue() as Int) + "/" + Self.GetSlotLimit() + ")")
  If PetPower.GetValue() > PetPowerMax.GetValue()
    Debug.MessageBox("Error: Pet power overflow, this should not happen at this point! Releasing new pet")
    Self.ReleasePet(petActor)
    Return
  EndIf
  Game.AdvanceSkill("Speechcraft", SpeechcraftExperienceGainTaming.GetValue())
  If !TimerIsRunning
    TimerIsRunning = True
    Self.RegisterForSingleUpdateGameTime(1.0)
  EndIf
EndFunction

Function ReleasePet(Actor akTarget)
{ This releases your pet and clears the pet slot. Will throw if the pet is an invalid race or does not exist in a pet slot. }
  akTarget.BlockActivation(True) ; #DEBUG_LINE_NO:288
  If PetREF[0].GetReference() == akTarget as ObjectReference ; #DEBUG_LINE_NO:290
    PetREF[0].Clear() ; #DEBUG_LINE_NO:291
    PetHungerREF[0].Clear() ; #DEBUG_LINE_NO:292
  ElseIf PetREF[1].GetReference() == akTarget as ObjectReference ; #DEBUG_LINE_NO:293
    PetREF[1].Clear() ; #DEBUG_LINE_NO:294
    PetHungerREF[1].Clear() ; #DEBUG_LINE_NO:295
  ElseIf PetREF[2].GetReference() == akTarget as ObjectReference ; #DEBUG_LINE_NO:296
    PetREF[2].Clear() ; #DEBUG_LINE_NO:297
    PetHungerREF[2].Clear() ; #DEBUG_LINE_NO:298
  Else
    Debug.MessageBox("Error: Pet does not exist in any slot") ; #DEBUG_LINE_NO:300
    Return  ; #DEBUG_LINE_NO:301
  EndIf
  Race originalRace = akTarget.GetRace() ; #DEBUG_LINE_NO:304
  Self.SyncPetCount()
  Debug.Trace("[Instincts+] released a pet; slots in use " + (PetCount.GetValue() as Int) + "/" + SlotLimit)
  PetPower.Mod(-akTarget.GetActorValue(PowerAVName)) ; #DEBUG_LINE_NO:307
  akTarget.Disable(True) ; #DEBUG_LINE_NO:309
  akTarget.Delete() ; #DEBUG_LINE_NO:310
  If (PetCount.GetValue() == 0 as Float) && TimerIsRunning ; #DEBUG_LINE_NO:312
    TimerIsRunning = False ; #DEBUG_LINE_NO:313
    Self.UnregisterForUpdateGameTime() ; #DEBUG_LINE_NO:314
  EndIf
EndFunction

Int Function GetSlotLimit()
  If SlotLimit < 1
    Return 3
  EndIf
  Return SlotLimit
EndFunction

Int Function GetBigPower()
  If BigPower < 1
    Return 6
  EndIf
  Return BigPower
EndFunction

Int Function GetHugePower()
  If HugePower < 1
    Return 10
  EndIf
  Return HugePower
EndFunction

Int Function LevelNeededForPower(Int aiPower)
{ Player level needed: nothing for ordinary animals, more and more for the strong ones. }
  Int big = LevelBig
  Int great = LevelGreat
  Int huge = LevelHuge
  If big < 1
    big = 20
  EndIf
  If great < 1
    great = 30
  EndIf
  If huge < 1
    huge = 40
  EndIf
  If aiPower >= Self.GetHugePower()
    Return huge
  ElseIf aiPower >= 8
    Return great
  ElseIf aiPower >= Self.GetBigPower()
    Return big
  EndIf
  Return 0
EndFunction

Int Function SlotCostForPower(Int aiPower)
{ Slots an animal takes: 1 normally, 2 for strong ones (bears, big cats), 3 for the biggest (mammoth). }
  If aiPower >= Self.GetHugePower()
    Return 3
  ElseIf aiPower >= Self.GetBigPower()
    Return 2
  EndIf
  Return 1
EndFunction

Int Function SlotsInUse()
{ Slots held by the pets that exist now (read from the pets themselves, so it can never drift). }
  Int used = 0
  Int i = 0
  While i < PetREF.Length
    Actor pet = PetREF[i].GetReference() as Actor
    If pet
      used += Self.SlotCostForPower(pet.GetActorValue(PowerAVName) as Int)
    EndIf
    i += 1
  EndWhile
  Return used
EndFunction

Function SyncPetCount()
  PetCount.SetValue(Self.SlotsInUse() as Float)
EndFunction

Event OnUpdateGameTime()
  Actor pet1 = PetREF[0].GetReference() as Actor ; #DEBUG_LINE_NO:319
  Actor pet2 = PetREF[1].GetReference() as Actor ; #DEBUG_LINE_NO:320
  Actor pet3 = PetREF[2].GetReference() as Actor ; #DEBUG_LINE_NO:321
  If pet1 ; #DEBUG_LINE_NO:322
    ((Self as Quest) as mumirnik_quest_instincts_petstats).UpdatePetStatsGameTime(pet1) ; #DEBUG_LINE_NO:323
  EndIf
  If pet2 ; #DEBUG_LINE_NO:325
    ((Self as Quest) as mumirnik_quest_instincts_petstats).UpdatePetStatsGameTime(pet2) ; #DEBUG_LINE_NO:326
  EndIf
  If pet3 ; #DEBUG_LINE_NO:328
    ((Self as Quest) as mumirnik_quest_instincts_petstats).UpdatePetStatsGameTime(pet3) ; #DEBUG_LINE_NO:329
  EndIf
  Self.RegisterForSingleUpdateGameTime(1.0) ; #DEBUG_LINE_NO:331
EndEvent

ReferenceAlias Function GetRefForActor(Actor akTarget)
{ Returns the pet slot REF that is assocated with this actor. Throws if target is not in any pet slot. }
  If PetREF[0].GetReference() == akTarget as ObjectReference ; #DEBUG_LINE_NO:336
    Return PetREF[0] ; #DEBUG_LINE_NO:337
  ElseIf PetREF[1].GetReference() == akTarget as ObjectReference ; #DEBUG_LINE_NO:338
    Return PetREF[1] ; #DEBUG_LINE_NO:339
  ElseIf PetREF[2].GetReference() == akTarget as ObjectReference ; #DEBUG_LINE_NO:340
    Return PetREF[2] ; #DEBUG_LINE_NO:341
  Else
    Debug.MessageBox("Error: Cannot find ref for actor because target does not belong to any known pet slot") ; #DEBUG_LINE_NO:343
    Return None ; #DEBUG_LINE_NO:344
  EndIf
EndFunction

ReferenceAlias Function GetHungerRefForActor(Actor akTarget)
{ Returns the hunger REF that is assocated with this actor. Throws if target is not in any pet slot. }
  If PetREF[0].GetReference() == akTarget as ObjectReference ; #DEBUG_LINE_NO:350
    Return PetHungerREF[0] ; #DEBUG_LINE_NO:351
  ElseIf PetREF[1].GetReference() == akTarget as ObjectReference ; #DEBUG_LINE_NO:352
    Return PetHungerREF[1] ; #DEBUG_LINE_NO:353
  ElseIf PetREF[2].GetReference() == akTarget as ObjectReference ; #DEBUG_LINE_NO:354
    Return PetHungerREF[2] ; #DEBUG_LINE_NO:355
  Else
    Debug.MessageBox("Error: Cannot find hunger ref for actor because target does not belong to any known pet slot") ; #DEBUG_LINE_NO:357
    Return None ; #DEBUG_LINE_NO:358
  EndIf
EndFunction

ReferenceAlias Function GetNameRefForActor(Actor akTarget)
{ Returns the name REF that is assocated with this actor. Throws if target is not in any pet slot. }
  If PetREF[0].GetReference() == akTarget as ObjectReference ; #DEBUG_LINE_NO:364
    Return PetNameREF[0] ; #DEBUG_LINE_NO:365
  ElseIf PetREF[1].GetReference() == akTarget as ObjectReference ; #DEBUG_LINE_NO:366
    Return PetNameREF[1] ; #DEBUG_LINE_NO:367
  ElseIf PetREF[2].GetReference() == akTarget as ObjectReference ; #DEBUG_LINE_NO:368
    Return PetNameREF[2] ; #DEBUG_LINE_NO:369
  Else
    Debug.MessageBox("Error: Cannot find name ref for actor because target does not belong to any known pet slot") ; #DEBUG_LINE_NO:371
    Return None ; #DEBUG_LINE_NO:372
  EndIf
EndFunction

Int Function GetSlotNumberForActor(Actor akTarget)
{ Returns the pet slot number that is assocated with this actor. Throws if target is not in any pet slot. }
  If PetREF[0].GetReference() == akTarget as ObjectReference ; #DEBUG_LINE_NO:378
    Return 0 ; #DEBUG_LINE_NO:379
  ElseIf PetREF[1].GetReference() == akTarget as ObjectReference ; #DEBUG_LINE_NO:380
    Return 1 ; #DEBUG_LINE_NO:381
  ElseIf PetREF[2].GetReference() == akTarget as ObjectReference ; #DEBUG_LINE_NO:382
    Return 2 ; #DEBUG_LINE_NO:383
  Else
    Debug.MessageBox("Error: Cannot find slot number for actor because target does not belong to any known pet slot") ; #DEBUG_LINE_NO:385
    Return -1 ; #DEBUG_LINE_NO:386
  EndIf
EndFunction

Function UpdateHungerAbilities(Actor akTarget)
{ Updates the target's hunger abilities based on its current hunger. }
  String hungerAVName = ((Self as Quest) as mumirnik_quest_instincts_petstats).hungerAVName ; #DEBUG_LINE_NO:392
  Float hunger = akTarget.GetActorValue(hungerAVName) ; #DEBUG_LINE_NO:393
  If hunger >= 90 as Float ; #DEBUG_LINE_NO:394
    akTarget.AddSpell(HungerBuffSpell[0], False) ; #DEBUG_LINE_NO:395
    akTarget.RemoveSpell(HungerBuffSpell[1]) ; #DEBUG_LINE_NO:396
    akTarget.RemoveSpell(HungerBuffSpell[2]) ; #DEBUG_LINE_NO:397
    akTarget.RemoveSpell(HungerBuffSpell[3]) ; #DEBUG_LINE_NO:398
    akTarget.RemoveSpell(HungerBuffSpell[4]) ; #DEBUG_LINE_NO:399
    akTarget.RemoveSpell(HungerBuffSpell[5]) ; #DEBUG_LINE_NO:400
  ElseIf hunger >= 70 as Float ; #DEBUG_LINE_NO:401
    akTarget.RemoveSpell(HungerBuffSpell[0]) ; #DEBUG_LINE_NO:402
    akTarget.AddSpell(HungerBuffSpell[1], False) ; #DEBUG_LINE_NO:403
    akTarget.RemoveSpell(HungerBuffSpell[2]) ; #DEBUG_LINE_NO:404
    akTarget.RemoveSpell(HungerBuffSpell[3]) ; #DEBUG_LINE_NO:405
    akTarget.RemoveSpell(HungerBuffSpell[4]) ; #DEBUG_LINE_NO:406
    akTarget.RemoveSpell(HungerBuffSpell[5]) ; #DEBUG_LINE_NO:407
  ElseIf hunger >= 50 as Float ; #DEBUG_LINE_NO:408
    akTarget.RemoveSpell(HungerBuffSpell[0]) ; #DEBUG_LINE_NO:409
    akTarget.RemoveSpell(HungerBuffSpell[1]) ; #DEBUG_LINE_NO:410
    akTarget.AddSpell(HungerBuffSpell[2], False) ; #DEBUG_LINE_NO:411
    akTarget.RemoveSpell(HungerBuffSpell[3]) ; #DEBUG_LINE_NO:412
    akTarget.RemoveSpell(HungerBuffSpell[4]) ; #DEBUG_LINE_NO:413
    akTarget.RemoveSpell(HungerBuffSpell[5]) ; #DEBUG_LINE_NO:414
  ElseIf hunger >= 30 as Float ; #DEBUG_LINE_NO:415
    akTarget.RemoveSpell(HungerBuffSpell[0]) ; #DEBUG_LINE_NO:416
    akTarget.RemoveSpell(HungerBuffSpell[1]) ; #DEBUG_LINE_NO:417
    akTarget.RemoveSpell(HungerBuffSpell[2]) ; #DEBUG_LINE_NO:418
    akTarget.AddSpell(HungerBuffSpell[3], False) ; #DEBUG_LINE_NO:419
    akTarget.RemoveSpell(HungerBuffSpell[4]) ; #DEBUG_LINE_NO:420
    akTarget.RemoveSpell(HungerBuffSpell[5]) ; #DEBUG_LINE_NO:421
  ElseIf hunger >= 10 as Float ; #DEBUG_LINE_NO:422
    akTarget.RemoveSpell(HungerBuffSpell[0]) ; #DEBUG_LINE_NO:423
    akTarget.RemoveSpell(HungerBuffSpell[1]) ; #DEBUG_LINE_NO:424
    akTarget.RemoveSpell(HungerBuffSpell[2]) ; #DEBUG_LINE_NO:425
    akTarget.RemoveSpell(HungerBuffSpell[3]) ; #DEBUG_LINE_NO:426
    akTarget.AddSpell(HungerBuffSpell[4], False) ; #DEBUG_LINE_NO:427
    akTarget.RemoveSpell(HungerBuffSpell[5]) ; #DEBUG_LINE_NO:428
  Else
    akTarget.RemoveSpell(HungerBuffSpell[0]) ; #DEBUG_LINE_NO:430
    akTarget.RemoveSpell(HungerBuffSpell[1]) ; #DEBUG_LINE_NO:431
    akTarget.RemoveSpell(HungerBuffSpell[2]) ; #DEBUG_LINE_NO:432
    akTarget.RemoveSpell(HungerBuffSpell[3]) ; #DEBUG_LINE_NO:433
    akTarget.RemoveSpell(HungerBuffSpell[4]) ; #DEBUG_LINE_NO:434
    akTarget.AddSpell(HungerBuffSpell[5], False) ; #DEBUG_LINE_NO:435
  EndIf
EndFunction

Function RefreshHungerDisplay(Actor akTarget)
{ Refreshes the target's hunger display. Does not update the actual hunger stat. }
  ReferenceAlias ThisPetHungerREF = Self.GetHungerRefForActor(akTarget) ; #DEBUG_LINE_NO:441
  ObjectReference petHungerInstanceOld = ThisPetHungerREF.GetReference() ; #DEBUG_LINE_NO:443
  ThisPetHungerREF.Clear() ; #DEBUG_LINE_NO:444
  petHungerInstanceOld.Delete() ; #DEBUG_LINE_NO:445
  String hungerAVName = ((Self as Quest) as mumirnik_quest_instincts_petstats).hungerAVName ; #DEBUG_LINE_NO:447
  Int hunger = akTarget.GetActorValue(hungerAVName) as Int ; #DEBUG_LINE_NO:448
  Activator petHungerType = None ; #DEBUG_LINE_NO:449
  If hunger >= 90 ; #DEBUG_LINE_NO:450
    petHungerType = PetHungerActivator[0] ; #DEBUG_LINE_NO:451
  ElseIf hunger >= 70 ; #DEBUG_LINE_NO:452
    petHungerType = PetHungerActivator[1] ; #DEBUG_LINE_NO:453
  ElseIf hunger >= 50 ; #DEBUG_LINE_NO:454
    petHungerType = PetHungerActivator[2] ; #DEBUG_LINE_NO:455
  ElseIf hunger >= 30 ; #DEBUG_LINE_NO:456
    petHungerType = PetHungerActivator[3] ; #DEBUG_LINE_NO:457
  ElseIf hunger >= 10 ; #DEBUG_LINE_NO:458
    petHungerType = PetHungerActivator[4] ; #DEBUG_LINE_NO:459
  Else
    petHungerType = PetHungerActivator[5] ; #DEBUG_LINE_NO:461
  EndIf
  ObjectReference petHungerInstance = PlayerREF.PlaceAtMe(petHungerType as Form, 1, False, False) ; #DEBUG_LINE_NO:463
  ThisPetHungerREF.ForceRefTo(petHungerInstance) ; #DEBUG_LINE_NO:464
EndFunction

Function SetBaseNameDisplay(Actor akTarget, Int aiGender)
{ Sets the target's name to its base name. }
  akTarget.SetActorValue(GenderAVName, aiGender as Float) ; #DEBUG_LINE_NO:469
  ReferenceAlias ThisPetNameREF = Self.GetNameRefForActor(akTarget) ; #DEBUG_LINE_NO:471
  ObjectReference petNameInstance = Game.GetPlayer().PlaceAtMe(PetNameBaseActivator as Form, 1, False, False) ; #DEBUG_LINE_NO:472
  ThisPetNameREF.ForceRefTo(petNameInstance) ; #DEBUG_LINE_NO:473
EndFunction

Function RerollNameDisplay(Actor akTarget)
{ Refreshes the target's name display randomly. }
  ReferenceAlias ThisPetNameREF = Self.GetNameRefForActor(akTarget) ; #DEBUG_LINE_NO:478
  ObjectReference petNameInstanceOld = ThisPetNameREF.GetReference() ; #DEBUG_LINE_NO:480
  ThisPetNameREF.Clear() ; #DEBUG_LINE_NO:481
  petNameInstanceOld.Delete() ; #DEBUG_LINE_NO:482
  Activator petNameType = None ; #DEBUG_LINE_NO:484
  Int gender = akTarget.GetActorValue(GenderAVName) as Int ; #DEBUG_LINE_NO:485
  If !akTarget.HasKeyword(IsUnaggressivePetRaceKeyword) ; #DEBUG_LINE_NO:486
    If gender == 1 ; #DEBUG_LINE_NO:487
      petNameType = PetNameM.GetAt(Utility.RandomInt(0, PetNameM.GetSize() - 1)) as Activator ; #DEBUG_LINE_NO:488
    ElseIf gender == 2 ; #DEBUG_LINE_NO:489
      petNameType = PetNameF.GetAt(Utility.RandomInt(0, PetNameM.GetSize() - 1)) as Activator ; #DEBUG_LINE_NO:490
    Else
      Debug.MessageBox("Error: Target has invalid gender") ; #DEBUG_LINE_NO:492
      Return  ; #DEBUG_LINE_NO:493
    EndIf
  ElseIf gender == 1 ; #DEBUG_LINE_NO:496
    petNameType = PetNameUnaggressiveM.GetAt(Utility.RandomInt(0, PetNameM.GetSize() - 1)) as Activator ; #DEBUG_LINE_NO:497
  ElseIf gender == 2 ; #DEBUG_LINE_NO:498
    petNameType = PetNameUnaggressiveF.GetAt(Utility.RandomInt(0, PetNameM.GetSize() - 1)) as Activator ; #DEBUG_LINE_NO:499
  Else
    Debug.MessageBox("Error: Target has invalid gender") ; #DEBUG_LINE_NO:501
    Return  ; #DEBUG_LINE_NO:502
  EndIf
  ObjectReference petNameInstance = Game.GetPlayer().PlaceAtMe(petNameType as Form, 1, False, False) ; #DEBUG_LINE_NO:505
  ThisPetNameREF.ForceRefTo(petNameInstance) ; #DEBUG_LINE_NO:506
EndFunction

Int Function GetPowerForRaceId(Int aiRaceId)
{ Returns the power for a given race ID. }
  Return (PetPowerValuesList.GetAt(aiRaceId) as GlobalVariable).GetValue() as Int ; #DEBUG_LINE_NO:511
EndFunction

Int Function GetRaceIdForRace(Race akRace)
{ Returns the original race ID for a given actor race. }
  Int originalRaceId = -1 ; #DEBUG_LINE_NO:516
  originalRaceId = OriginalRaceList.Find(akRace as Form) ; #DEBUG_LINE_NO:517
  If originalRaceId == -1 ; #DEBUG_LINE_NO:518
    Debug.MessageBox("Error: Unknown actor race") ; #DEBUG_LINE_NO:519
  EndIf
  Return originalRaceId ; #DEBUG_LINE_NO:521
EndFunction

Int Function GetRaceIdForTamedAnimal(Actor akTarget)
  Int actorId = NewRaceList.Find(akTarget.GetRace() as Form) ; #DEBUG_LINE_NO:525
  If actorId == -1 ; #DEBUG_LINE_NO:526
    Debug.MessageBox("Error: Unknown actor") ; #DEBUG_LINE_NO:527
    Return -1 ; #DEBUG_LINE_NO:528
  EndIf
  Return actorId ; #DEBUG_LINE_NO:530
EndFunction

Function TurnToPlayer(Actor akTarget)
{ Turns the target to face the player. }
  Float zOffset = akTarget.GetHeadingAngle(PlayerREF as ObjectReference) ; #DEBUG_LINE_NO:535
  akTarget.SetAngle(akTarget.GetAngleX(), akTarget.GetAngleY(), akTarget.GetAngleZ() + zOffset) ; #DEBUG_LINE_NO:536
EndFunction
