ScriptName Mumirnik_Quest_Instincts_InjectCC Extends Quest
{ Adds CC items to the food lists. This is mostly hardcoded, while a softcoded solution can be found in the repository, the large number of both formlists and items makes it impractical in this case. }

;-- Variables ---------------------------------------
Int added = 0

;-- Properties --------------------------------------
FormList Property DestinationFormListEverything Auto
FormList Property DestinationFormListFish Auto
FormList Property DestinationFormListMeat Auto
FormList Property DestinationFormListMeatFish Auto
FormList Property DestinationFormListVegetables Auto
FormList Property DestinationFormListVegetablesMeatFish Auto
Float Property RunDelay Auto
{ This is used in some other people's scripts, I assume it prevents the cart glitch. }

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Event OnInit()
  Self.RegisterForSingleUpdate(RunDelay) ; #DEBUG_LINE_NO:19
EndEvent

Event OnUpdate()
  Debug.Trace("Instincts: Start CC injection", 0) ; #DEBUG_LINE_NO:23
  Self.AddFishing() ; #DEBUG_LINE_NO:24
  Self.AddCurios() ; #DEBUG_LINE_NO:25
  Self.AddCause() ; #DEBUG_LINE_NO:26
  Self.AddZombies() ; #DEBUG_LINE_NO:27
  Self.AddPuzzleDungeon() ; #DEBUG_LINE_NO:28
  Debug.Trace(("Instincts: Finish CC injection, added " + added as String) + " items", 0) ; #DEBUG_LINE_NO:29
EndEvent

Function AddFishing()
  Form item = None ; #DEBUG_LINE_NO:33
  item = Game.GetFormFromFile(2283, "ccBGSSSE001-Fish.esm") ; #DEBUG_LINE_NO:34
  If !item ; #DEBUG_LINE_NO:35
    Debug.Trace("Instincts: Fishing not installed", 0) ; #DEBUG_LINE_NO:36
    Return  ; #DEBUG_LINE_NO:37
  EndIf
  Self.AddToFormListsFish(Game.GetFormFromFile(2283, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:39
  Self.AddToFormListsFish(Game.GetFormFromFile(2284, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:40
  Self.AddToFormListsFish(Game.GetFormFromFile(2285, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:41
  Self.AddToFormListsFish(Game.GetFormFromFile(2286, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:42
  Self.AddToFormListsFish(Game.GetFormFromFile(2287, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:43
  Self.AddToFormListsFish(Game.GetFormFromFile(2288, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:44
  Self.AddToFormListsFish(Game.GetFormFromFile(2289, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:45
  Self.AddToFormListsFish(Game.GetFormFromFile(2290, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:46
  Self.AddToFormListsFish(Game.GetFormFromFile(2291, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:47
  Self.AddToFormListsFish(Game.GetFormFromFile(2170, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:48
  Self.AddToFormListsFish(Game.GetFormFromFile(2171, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:49
  Self.AddToFormListsFish(Game.GetFormFromFile(2172, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:50
  Self.AddToFormListsFish(Game.GetFormFromFile(2173, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:51
  Self.AddToFormListsFish(Game.GetFormFromFile(2174, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:52
  Self.AddToFormListsFish(Game.GetFormFromFile(2175, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:53
  Self.AddToFormListsFish(Game.GetFormFromFile(2176, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:54
  Self.AddToFormListsFish(Game.GetFormFromFile(2177, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:55
  Self.AddToFormListsFish(Game.GetFormFromFile(2178, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:56
  Self.AddToFormListsFish(Game.GetFormFromFile(2179, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:57
  Self.AddToFormListsFish(Game.GetFormFromFile(2180, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:58
  Self.AddToFormListsFish(Game.GetFormFromFile(2181, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:59
  Self.AddToFormListsFish(Game.GetFormFromFile(2182, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:60
  Self.AddToFormListsFish(Game.GetFormFromFile(2183, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:61
  Self.AddToFormListsFish(Game.GetFormFromFile(2184, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:62
  Self.AddToFormListsFish(Game.GetFormFromFile(2186, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:63
  Self.AddToFormListsFish(Game.GetFormFromFile(2187, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:64
  Self.AddToFormListsFish(Game.GetFormFromFile(2188, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:65
  Self.AddToFormListsFish(Game.GetFormFromFile(2189, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:66
  Self.AddToFormListsFish(Game.GetFormFromFile(2190, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:67
  Self.AddToFormListsFish(Game.GetFormFromFile(2191, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:68
  Self.AddToFormListsFish(Game.GetFormFromFile(2192, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:69
  Self.AddToFormListsFish(Game.GetFormFromFile(2193, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:70
  Self.AddToFormListsFish(Game.GetFormFromFile(2198, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:71
  Self.AddToFormListsFish(Game.GetFormFromFile(2199, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:72
  Self.AddToFormListsFish(Game.GetFormFromFile(2200, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:73
  Self.AddToFormListsFish(Game.GetFormFromFile(2202, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:74
  Self.AddToFormListsFish(Game.GetFormFromFile(2203, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:75
  Self.AddToFormListsFish(Game.GetFormFromFile(2204, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:76
  Self.AddToFormListsFish(Game.GetFormFromFile(2206, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:77
  Self.AddToFormListsFish(Game.GetFormFromFile(2208, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:78
  Self.AddToFormListsFish(Game.GetFormFromFile(2209, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:79
  Self.AddToFormListsFish(Game.GetFormFromFile(2210, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:80
  Self.AddToFormListsFish(Game.GetFormFromFile(2211, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:81
  Self.AddToFormListsFish(Game.GetFormFromFile(2212, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:82
  Self.AddToFormListsFish(Game.GetFormFromFile(2213, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:83
  Self.AddToFormListsFish(Game.GetFormFromFile(2999, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:84
  Self.AddToFormListsFish(Game.GetFormFromFile(3117, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:85
  Self.AddToFormListsFish(Game.GetFormFromFile(3838, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:86
  Self.AddToFormListsFish(Game.GetFormFromFile(3842, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:87
  Self.AddToFormListsFish(Game.GetFormFromFile(3844, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:88
  Self.AddToFormListsFish(Game.GetFormFromFile(3846, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:89
  Self.AddToFormListsFish(Game.GetFormFromFile(3850, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:90
  Self.AddToFormListsFish(Game.GetFormFromFile(3851, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:91
  Self.AddToFormListsFish(Game.GetFormFromFile(3852, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:92
  Self.AddToFormListsFish(Game.GetFormFromFile(3877, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:93
  Self.AddToFormListsFish(Game.GetFormFromFile(3958, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:94
  Self.AddToFormListsFish(Game.GetFormFromFile(3959, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:95
  Self.AddToFormListsFish(Game.GetFormFromFile(3960, "ccBGSSSE001-Fish.esm")) ; #DEBUG_LINE_NO:96
EndFunction

Function AddCurios()
  Form item = None ; #DEBUG_LINE_NO:100
  item = Game.GetFormFromFile(6146, "ccBGSSSE037-Curios.esl") ; #DEBUG_LINE_NO:101
  If !item ; #DEBUG_LINE_NO:102
    Debug.Trace("Instincts: Curios not installed", 0) ; #DEBUG_LINE_NO:103
    Return  ; #DEBUG_LINE_NO:104
  EndIf
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6146, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:106
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6148, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:107
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6149, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:108
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6150, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:109
  Self.AddToFormListsEverything(Game.GetFormFromFile(6151, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:110
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6153, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:111
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6154, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:112
  Self.AddToFormListsEverything(Game.GetFormFromFile(6155, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:113
  Self.AddToFormListsMeat(Game.GetFormFromFile(6156, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:114
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6157, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:115
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6158, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:116
  Self.AddToFormListsEverything(Game.GetFormFromFile(6159, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:117
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6161, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:118
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6163, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:119
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6164, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:120
  Self.AddToFormListsEverything(Game.GetFormFromFile(6165, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:121
  Self.AddToFormListsEverything(Game.GetFormFromFile(6166, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:122
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6167, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:123
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6168, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:124
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6169, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:125
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6170, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:126
  Self.AddToFormListsFish(Game.GetFormFromFile(6171, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:127
  Self.AddToFormListsMeat(Game.GetFormFromFile(6172, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:128
  Self.AddToFormListsMeat(Game.GetFormFromFile(6173, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:129
  Self.AddToFormListsEverything(Game.GetFormFromFile(6174, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:130
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6175, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:131
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6177, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:132
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6178, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:133
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6179, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:134
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6180, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:135
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6181, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:136
  Self.AddToFormListsEverything(Game.GetFormFromFile(6182, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:137
  Self.AddToFormListsEverything(Game.GetFormFromFile(6183, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:138
  Self.AddToFormListsEverything(Game.GetFormFromFile(6184, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:139
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6185, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:140
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6186, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:141
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6187, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:142
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6188, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:143
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6189, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:144
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6190, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:145
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6198, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:146
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6199, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:147
  Self.AddToFormListsVegetables(Game.GetFormFromFile(6200, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:148
  Self.AddToFormListsVegetables(Game.GetFormFromFile(7522, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:149
  Self.AddToFormListsVegetables(Game.GetFormFromFile(7526, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:150
  Self.AddToFormListsVegetables(Game.GetFormFromFile(7528, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:151
  Self.AddToFormListsVegetables(Game.GetFormFromFile(7530, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:152
  Self.AddToFormListsVegetables(Game.GetFormFromFile(7532, "ccBGSSSE037-Curios.esl")) ; #DEBUG_LINE_NO:153
EndFunction

Function AddCause()
  Form item = None ; #DEBUG_LINE_NO:157
  item = Game.GetFormFromFile(150545, "ccBGSSSE067-daedinv.esm") ; #DEBUG_LINE_NO:158
  If !item ; #DEBUG_LINE_NO:159
    Debug.Trace("Instincts: Cause not installed", 0) ; #DEBUG_LINE_NO:160
    Return  ; #DEBUG_LINE_NO:161
  EndIf
  Self.AddToFormListsVegetables(Game.GetFormFromFile(150545, "ccBGSSSE067-daedinv.esm")) ; #DEBUG_LINE_NO:163
  Self.AddToFormListsVegetables(Game.GetFormFromFile(150547, "ccBGSSSE067-daedinv.esm")) ; #DEBUG_LINE_NO:164
  Self.AddToFormListsVegetables(Game.GetFormFromFile(150549, "ccBGSSSE067-daedinv.esm")) ; #DEBUG_LINE_NO:165
EndFunction

Function AddZombies()
  Form item = None ; #DEBUG_LINE_NO:169
  item = Game.GetFormFromFile(2060, "ccBGSSSE003-zombies.esl") ; #DEBUG_LINE_NO:170
  If !item ; #DEBUG_LINE_NO:171
    Debug.Trace("Instincts: Zombies not installed", 0) ; #DEBUG_LINE_NO:172
    Return  ; #DEBUG_LINE_NO:173
  EndIf
  Self.AddToFormListsMeat(Game.GetFormFromFile(2060, "ccBGSSSE003-zombies.esl")) ; #DEBUG_LINE_NO:175
EndFunction

Function AddPuzzleDungeon()
  Form item = None ; #DEBUG_LINE_NO:179
  item = Game.GetFormFromFile(105786, "cctwbsse001-puzzledungeon.esm") ; #DEBUG_LINE_NO:180
  If !item ; #DEBUG_LINE_NO:181
    Debug.Trace("Instincts: Puzzle Dungeon not installed", 0) ; #DEBUG_LINE_NO:182
    Return  ; #DEBUG_LINE_NO:183
  EndIf
  Self.AddToFormListsVegetables(Game.GetFormFromFile(105760, "cctwbsse001-puzzledungeon.esm")) ; #DEBUG_LINE_NO:185
  Self.AddToFormListsMeat(Game.GetFormFromFile(209514, "cctwbsse001-puzzledungeon.esm")) ; #DEBUG_LINE_NO:186
  Self.AddToFormListsMeat(Game.GetFormFromFile(105786, "cctwbsse001-puzzledungeon.esm")) ; #DEBUG_LINE_NO:187
EndFunction

Function AddToFormListsEverything(Form akItem)
  DestinationFormListEverything.AddForm(akItem) ; #DEBUG_LINE_NO:191
  added += 1 ; #DEBUG_LINE_NO:192
EndFunction

Function AddToFormListsVegetables(Form akItem)
  DestinationFormListEverything.AddForm(akItem) ; #DEBUG_LINE_NO:196
  DestinationFormListVegetables.AddForm(akItem) ; #DEBUG_LINE_NO:197
  DestinationFormListVegetablesMeatFish.AddForm(akItem) ; #DEBUG_LINE_NO:198
  added += 1 ; #DEBUG_LINE_NO:199
EndFunction

Function AddToFormListsMeat(Form akItem)
  DestinationFormListEverything.AddForm(akItem) ; #DEBUG_LINE_NO:203
  DestinationFormListMeat.AddForm(akItem) ; #DEBUG_LINE_NO:204
  DestinationFormListMeatFish.AddForm(akItem) ; #DEBUG_LINE_NO:205
  DestinationFormListVegetablesMeatFish.AddForm(akItem) ; #DEBUG_LINE_NO:206
  added += 1 ; #DEBUG_LINE_NO:207
EndFunction

Function AddToFormListsFish(Form akItem)
  DestinationFormListEverything.AddForm(akItem) ; #DEBUG_LINE_NO:211
  DestinationFormListFish.AddForm(akItem) ; #DEBUG_LINE_NO:212
  DestinationFormListMeatFish.AddForm(akItem) ; #DEBUG_LINE_NO:213
  DestinationFormListVegetablesMeatFish.AddForm(akItem) ; #DEBUG_LINE_NO:214
  added += 1 ; #DEBUG_LINE_NO:215
EndFunction
