sgs.ai_skill_invoke.shuijingh = true

sgs.ai_skill_cardask["@shuijingh"] = function(self)
	local condition = self.room:getTag("Shuijingh"):toString()
	local conditions = condition:split("+")
	local target = self.room:findPlayer(conditions[1])
	local suit = conditions[2]
	local reason = conditions[3]
	self.room:writeToConsole(reason)
	local invoke = false
	if self:isFriend(target) or target:objectName() == self.player:objectName() then
		if reason == "recover" and target:getLostHp() > 1 then invoke = true end
	else
		if reason == "damage" or reason == "lost" then invoke = true end
	end
	if invoke == false then return "." end
end

sgs.ai_skill_use["@@fenbeih"] = function(self, prompt)
	self:sort(self.enemies, "handcard")
	local target = self.enemies[#self.enemies]
	return ("@FenbeihCard=.->%s"):format(target:objectName())
end

sgs.ai_card_intention.FenbeihCard = 80

sgs.ai_skill_invoke.yindunh = true

function sgs.ai_filterskill_filter.fangshuh_buff(card, card_place, player)
	local fsid = -1
	for _, player in sgs.qlist(global_room:getAllPlayers()) do
		if not player:getPile("fangshuh_pile"):isEmpty() then fsid = player:getPile("fangshuh_pile"):first() end
	end
	if fsid ~= -1 then
		local fsc = sgs.Sanguosha:getCard(fsid)
		local name = fsc:objectName()
		local suit = fsc:getSuitString()
		local number = fsc:getNumberString()
	
		if card_place == sgs.Player_Hand then
			return ("%s:fangshuh_buff[%s:%s]=%d"):format(name, suit, number, card:getEffectiveId())
		end
	end
end