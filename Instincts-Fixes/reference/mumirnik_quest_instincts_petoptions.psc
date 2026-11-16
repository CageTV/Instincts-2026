ScriptName Mumirnik_Quest_Instincts_PetOptions Extends Quest
{ The guts of the pet system. Handles pet taming and releasing, and manages pet slots and the slot cap. }

;-- Variables ---------------------------------------
Actor AnimalToTame
Float TameChance = 0.0
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
  Race originalRace = akTarget.GetRace() ; #DEBUG_LINE_NO:111
  Int originalRaceId = Self.GetRaceIdForRace(originalRace) ; #DEBUG_LINE_NO:112
  Int power = Self.GetPowerForRaceId(originalRaceId) ; #DEBUG_LINE_NO:113
  If originalRaceId == -1 || power == -1 ; #DEBUG_LINE_NO:114
    Debug.Notification("Error: Invalid original race") ; #DEBUG_LINE_NO:115
  EndIf
  Int currentPetPower = PetPower.GetValue() as Int ; #DEBUG_LINE_NO:118
  Int currentPetPowerMax = PetPowerMax.GetValue() as Int ; #DEBUG_LINE_NO:119
  If currentPetPower + power > currentPetPowerMax ; #DEBUG_LINE_NO:120
    PetPowerCheckFAILED.Show(currentPetPower as Float, currentPetPowerMax as Float, power as Float, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0) ; #DEBUG_LINE_NO:121
    Return False ; #DEBUG_LINE_NO:122
  ElseIf PetCount.GetValue() == 3 as Float ; #DEBUG_LINE_NO:124
    PetCountCheckFAILED.Show(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0) ; #DEBUG_LINE_NO:125
    Return False ; #DEBUG_LINE_NO:126
  Else
    Return True ; #DEBUG_LINE_NO:128
  EndIf
EndFunction

Function TryMakePet(Actor akTarget, Float aiValue)
{ This is called from the taming feature. It is a thread safe way to make a target your pet. }
  AnimalToTame = akTarget ; #DEBUG_LINE_NO:135
  TameChance += aiValue ; #DEBUG_LINE_NO:136
  ChanceToTame.Show(TameChance, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0) ; #DEBUG_LINE_NO:137
  Self.RegisterForSingleUpdate(0.02) ; #DEBUG_LINE_NO:138
EndFunction

Event OnUpdate()
  If AnimalToTame ; #DEBUG_LINE_NO:142
    If Utility.RandomInt(0, 99) as Float < TameChance ; #DEBUG_LINE_NO:143
      Self.MakePet(AnimalToTame) ; #DEBUG_LINE_NO:144
    Else
      ChanceToTameFAILED.Show(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0) ; #DEBUG_LINE_NO:146
    EndIf
    AnimalToTame = None ; #DEBUG_LINE_NO:148
    TameChance = 0.0 ; #DEBUG_LINE_NO:149
  EndIf
EndEvent

Function MakePet(Actor akTarget)
{ This makes the target your pet, initializes its stats and starts the hunger timer. Will throw if the target is an invalid race or there are no free pet slots. }
  akTarget.SetAlpha(0.0, True) ; #DEBUG_LINE_NO:155
  akTarget.SetCriticalStage(akTarget.CritStage_DisintegrateEnd) ; #DEBUG_LINE_NO:156
  Race originalRace = akTarget.GetRace() ; #DEBUG_LINE_NO:158
  Int originalRaceId = Self.GetRaceIdForRace(originalRace) ; #DEBUG_LINE_NO:159
  Bool isForcedRandomGenderMaleVisuals = ForcedRandomGenderMaleVisualsRaceList.find(originalRace, 0) != -1 ; #DEBUG_LINE_NO:161
  Int gender = 0 ; #DEBUG_LINE_NO:163
  If ForcedFemaleRaceList.find(originalRace, 0) != -1 ; #DEBUG_LINE_NO:164
    gender = 2 ; #DEBUG_LINE_NO:165
  ElseIf ForcedMaleRaceList.find(originalRace, 0) != -1 ; #DEBUG_LINE_NO:166
    gender = 1 ; #DEBUG_LINE_NO:167
  ElseIf ForcedRandomGenderRaceList.find(originalRace, 0) != -1 || isForcedRandomGenderMaleVisuals ; #DEBUG_LINE_NO:168
    gender = Utility.RandomInt(1, 2) ; #DEBUG_LINE_NO:169
  Else
    gender = akTarget.GetLeveledActorBase().GetSex() + 1 ; #DEBUG_LINE_NO:171
  EndIf
  ActorBase newActorBase = None ; #DEBUG_LINE_NO:174
  If gender == 1 || isForcedRandomGenderMaleVisuals ; #DEBUG_LINE_NO:175
    newActorBase = PetActorListM.GetAt(originalRaceId) as ActorBase ; #DEBUG_LINE_NO:176
  ElseIf gender == 2 ; #DEBUG_LINE_NO:177
    newActorBase = PetActorListF.GetAt(originalRaceId) as ActorBase ; #DEBUG_LINE_NO:178
  Else
    Debug.MessageBox("Error: Unknown gender") ; #DEBUG_LINE_NO:180
  EndIf
  ActorBase targetActorBase = akTarget.GetLeveledActorBase() ; #DEBUG_LINE_NO:183
  Int raceSubstituteActorBaseId = RaceSubstituteOriginalActorBaseList.find(targetActorBase, 0) ; #DEBUG_LINE_NO:184
  If raceSubstituteActorBaseId == -1 ; #DEBUG_LINE_NO:185
    targetActorBase = akTarget.GetBaseObject() as ActorBase ; #DEBUG_LINE_NO:186
    raceSubstituteActorBaseId = RaceSubstituteOriginalActorBaseList.find(targetActorBase, 0) ; #DEBUG_LINE_NO:187
  EndIf
  Int targetActorBaseId = targetActorBase.GetFormID() ; #DEBUG_LINE_NO:191
  If targetActorBaseId == 67348929 && originalRace.GetFormID() == 78346 ; #DEBUG_LINE_NO:193
    raceSubstituteActorBaseId = 9 ; #DEBUG_LINE_NO:194
  ElseIf targetActorBaseId == 271029 && originalRace.GetFormID() == 78346 && akTarget.GetLevel() >= 6 ; #DEBUG_LINE_NO:195
    raceSubstituteActorBaseId = 9 ; #DEBUG_LINE_NO:196
  ElseIf targetActorBaseId == 271024 ; #DEBUG_LINE_NO:197
    Int originalRaceFormID = originalRace.GetFormID() ; #DEBUG_LINE_NO:198
    If originalRaceFormID == 78328 ; #DEBUG_LINE_NO:199
      raceSubstituteActorBaseId = 5 ; #DEBUG_LINE_NO:200
    ElseIf originalRaceFormID == 341111 ; #DEBUG_LINE_NO:201
      raceSubstituteActorBaseId = 3 ; #DEBUG_LINE_NO:202
    EndIf
  ElseIf targetActorBaseId == 124835 && originalRace.GetFormID() == 78346 && akTarget.GetLevel() >= 6 ; #DEBUG_LINE_NO:204
    raceSubstituteActorBaseId = 9 ; #DEBUG_LINE_NO:205
  ElseIf targetActorBaseId == 416315 && originalRace.GetFormID() == 78346 && akTarget.GetLevel() >= 6 ; #DEBUG_LINE_NO:206
    raceSubstituteActorBaseId = 9 ; #DEBUG_LINE_NO:207
  ElseIf targetActorBaseId == 969066 && originalRace.GetFormID() == 1088636 ; #DEBUG_LINE_NO:208
    raceSubstituteActorBaseId = 0 ; #DEBUG_LINE_NO:209
  EndIf
  If raceSubstituteActorBaseId != -1 ; #DEBUG_LINE_NO:212
    If gender == 1 || isForcedRandomGenderMaleVisuals ; #DEBUG_LINE_NO:213
      newActorBase = RaceSubstituteActorListM[raceSubstituteActorBaseId] ; #DEBUG_LINE_NO:214
      originalRaceId = PetActorListM.Find(newActorBase as Form) ; #DEBUG_LINE_NO:215
      If originalRaceId == -1 ; #DEBUG_LINE_NO:216
        Debug.MessageBox("Error: No original race for this race substitution") ; #DEBUG_LINE_NO:217
      EndIf
    ElseIf gender == 2 ; #DEBUG_LINE_NO:219
      newActorBase = RaceSubstituteActorListF[raceSubstituteActorBaseId] ; #DEBUG_LINE_NO:220
      originalRaceId = PetActorListF.Find(newActorBase as Form) ; #DEBUG_LINE_NO:221
      If originalRaceId == -1 ; #DEBUG_LINE_NO:222
        Debug.MessageBox("Error: No original race for this race substitution") ; #DEBUG_LINE_NO:223
      EndIf
    Else
      Debug.MessageBox("Error: Unknown gender") ; #DEBUG_LINE_NO:226
    EndIf
  EndIf
  Actor petActor = akTarget.PlaceActorAtMe(newActorBase, 4, None) ; #DEBUG_LINE_NO:230
  petActor.MoveTo(akTarget as ObjectReference, 0.0, 0.0, 0.0, True) ; #DEBUG_LINE_NO:231
  TamedSound.Play(petActor as ObjectReference) ; #DEBUG_LINE_NO:233
  Int slotFilled = -1 ; #DEBUG_LINE_NO:235
  If PetREF[0].GetReference() == None ; #DEBUG_LINE_NO:236
    PetREF[0].ForceRefTo(petActor as ObjectReference) ; #DEBUG_LINE_NO:237
    slotFilled = 0 ; #DEBUG_LINE_NO:238
  ElseIf PetREF[1].GetReference() == None ; #DEBUG_LINE_NO:239
    PetREF[1].ForceRefTo(petActor as ObjectReference) ; #DEBUG_LINE_NO:240
    slotFilled = 1 ; #DEBUG_LINE_NO:241
  ElseIf PetREF[2].GetReference() == None ; #DEBUG_LINE_NO:242
    PetREF[2].ForceRefTo(petActor as ObjectReference) ; #DEBUG_LINE_NO:243
    slotFilled = 2 ; #DEBUG_LINE_NO:244
  Else
    Debug.MessageBox(("Error: No free pet slot, PetCount = " + (PetCount.GetValue() as Int) as String) + ", reverting") ; #DEBUG_LINE_NO:246
    petActor.Delete() ; #DEBUG_LINE_NO:247
    Return  ; #DEBUG_LINE_NO:248
  EndIf
  akTarget.Kill(PlayerREF) ; #DEBUG_LINE_NO:251
  petActor.StopCombat() ; #DEBUG_LINE_NO:253
  petActor.SetPlayerTeammate(True, False) ; #DEBUG_LINE_NO:254
  petActor.EvaluatePackage() ; #DEBUG_LINE_NO:255
  TamedMessage[slotFilled].Show(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0) ; #DEBUG_LINE_NO:257
  petActor.RestoreActorValue("Health", 10000 as Float) ; #DEBUG_LINE_NO:259
  ((Self as Quest) as mumirnik_quest_instincts_petstats).SetHunger(petActor, 50 as Float) ; #DEBUG_LINE_NO:260
  ((Self as Quest) as mumirnik_quest_instincts_pettraining).InitTrainingProgression(petActor, originalRaceId) ; #DEBUG_LINE_NO:261
  Self.SetBaseNameDisplay(petActor, gender) ; #DEBUG_LINE_NO:263
  PetCount.Mod(1 as Float) ; #DEBUG_LINE_NO:265
  Int power = Self.GetPowerForRaceId(originalRaceId) ; #DEBUG_LINE_NO:266
  PetPower.Mod(power as Float) ; #DEBUG_LINE_NO:267
  petActor.SetActorValue(PowerAVName, power as Float) ; #DEBUG_LINE_NO:268
  If PetPower.GetValue() > PetPowerMax.GetValue() ; #DEBUG_LINE_NO:270
    Debug.MessageBox("Error: Pet power overflow, this should not happen at this point! Releasing new pet") ; #DEBUG_LINE_NO:271
    Self.ReleasePet(petActor) ; #DEBUG_LINE_NO:272
    Return  ; #DEBUG_LINE_NO:273
  EndIf
  Game.AdvanceSkill("Speechcraft", SpeechcraftExperienceGainTaming.GetValue()) ; #DEBUG_LINE_NO:278
  If !TimerIsRunning ; #DEBUG_LINE_NO:280
    TimerIsRunning = True ; #DEBUG_LINE_NO:281
    Self.RegisterForSingleUpdateGameTime(1.0) ; #DEBUG_LINE_NO:282
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
  PetCount.Mod(-1 as Float) ; #DEBUG_LINE_NO:306
  PetPower.Mod(-akTarget.GetActorValue(PowerAVName)) ; #DEBUG_LINE_NO:307
  akTarget.Disable(True) ; #DEBUG_LINE_NO:309
  akTarget.Delete() ; #DEBUG_LINE_NO:310
  If (PetCount.GetValue() == 0 as Float) && TimerIsRunning ; #DEBUG_LINE_NO:312
    TimerIsRunning = False ; #DEBUG_LINE_NO:313
    Self.UnregisterForUpdateGameTime() ; #DEBUG_LINE_NO:314
  EndIf
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
