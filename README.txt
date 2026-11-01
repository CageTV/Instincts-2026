Instincts - Fixes (for Instincts - Animal Taming in Skyrim 1.1.6, Nexus 63224, by Kheree; the mod's permissions allow modification)

1. Summon Companions never summoned the last pet. Mumirnik_Effect_SummonREFs looped `While i < ActorREFs.Length - 1`; ActorREFs holds the three pet
   slots, so only two pets came. The loop now runs to Length.
