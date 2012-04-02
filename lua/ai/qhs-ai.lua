sgs.ai_skill_invoke.shuijingh = true

sgs.ai_skill_cardask["@shuijingh"] = function(self)
	local target = self.room:getTag("ShuijinghTarget"):toPlayer()
	local invoke = false
	if self:isFriend(target) then
		if target:hasFlag("recover") and target:getLostHp() > 1 then invoke = true end
	else
		if target:hasFlag("damage") or target:hasFlag("lost") then invoke = true end
	end
	if invoke == false then return "." end
end

sgs.ai_skill_use["@@fenbeih"] = function(self, prompt)
	self:sort(self.enemies, "handcard")
	local target = self.enemies[#self.enemies]:objectName()
	return ("@FenbeihCard=.->%s+%s"):format(target)
end

sgs.ai_card_intention.FenbeihCard = 80

sgs.ai_skill_invoke.yindunh = true

