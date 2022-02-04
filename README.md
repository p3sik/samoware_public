
## Lua api functions:
AddHook(string name, string id, callback)
RemoveHook(string name, string id)

StartPrediction(CUserCmd cmd)
FinishPrediction()

StartSimulation(number playerIndex, table cmdData)
SimulateTick(number playerIndex, number numTick, table cmdData)
FinishSimulation(number playerIndex)

PredictSpread(CUserCmd cmd, Angle angle, number weapSpread) -> Vector

GetServerTime(CUserCmd cmd) -> number

SetCommandTick(CUserCmd cmd, number tickCount)

SetTyping(CUserCmd cmd, bool isTyping)

GetSimulationTime(number playerIndex) -> number

GetLatency(number flow) -> number

GetTargetLBY(number playerIndex) -> number

GetCurrentLBY(number playerIndex) -> number

Get/SetInSequenceNr() -> number
Get/SetOutSequenceNr() -> number
Get/SetOutSequenceNrAck() -> number
Get/SetNetChokedPackets() -> number

Get/SetLastCommandAck() -> number
Get/SetLastOutgoingCommand() -> number
Get/SetChokedCommands() -> number

NetSetConVar(string name, string value)

SetShouldChoke(bool shouldChoke)
SetForceChoke(bool shouldChoke)
