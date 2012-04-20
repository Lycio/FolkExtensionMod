local yjjubing_skill = {}
yjjubing_skill.name = "yjjubing"
table.insert(sgs.ai_skills, yjjubing_skill)
yjjubing_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("YjJubingCard") then return end
	local card_str = "@YjJubingCard=."
	local card = sgs.Card_Parse(card_str)
	assert(card)
	return card
end

sgs.ai_skill_use_func.YjJubingCard = function(card,use,self)
	use.card = card
	return		
end

sgs.ai_use_priority.YjJubingCard = 4.2

sgs.ai_skill_choice.yjzhufang = function(self, choices)
	if not self.player:isWounded() then return "draw" 
	elseif self.player:hasSkill("jijiu") or self:getCardsNum("Peach") > 0 then return "draw"
	else return "recover" end	
end

local yjyoujin_skill = {}
yjyoujin_skill.name = "yjyoujin"
table.insert(sgs.ai_skills, yjyoujin_skill)
yjyoujin_skill.getTurnUseCard = function(self)
	local cards = self.player:getCards("he")
	local ids = sgs.IntList()
	for _,cd in sgs.qlist(cards) do
		if cd:inherits("Slash") or cd:inherits("EquipCard") then
			ids:append(cd:getEffectiveId())
		end
	end
	if ids:isEmpty() or self.player:hasUsed("YjYoujinCard") then return nil end
	
	local card = sgs.Sanguosha:getCard(ids:first())
	if not card then return nil end
	local card_id = card:getEffectiveId()
	local card_str = "@YjYoujinCard="..card_id
	local acard = sgs.Card_Parse(card_str)
	assert(acard)
	return acard
end

sgs.ai_skill_use_func.YjYoujinCard = function(card,use,self)
	self:sort(self.friends_noself, "defense")
	for _, friend in ipairs(self.friends_noself) do
		if not friend:faceUp() then
			use.card = card
			if use.to then use.to:append(friend) end
			return
		end
	end
	if #self.friends_noself > 0 then
		local acard = sgs.Sanguosha:getCard(card:getSubcards():first())
		if acard:inherits("EquipCard") then
			use.card = card
			if use.to then use.to:append(self.friends_noself[1]) end
			return
		elseif acard:inherits("Slash") then
			use.card = card
			if use.to then use.to:append(self.friends_noself[#self.friends_noself]) end
			return
		end
	else
		self:sort(self.enemies, "handcard")
		use.card = card
		if use.to then use.to:append(self.enemies[1]) end
		return
	end
end

local yjrangli_skill = {}
yjrangli_skill.name = "yjrangli"
table.insert(sgs.ai_skills, yjrangli_skill)
yjrangli_skill.getTurnUseCard = function(self)
	local friends = self.friends_noself 
	local x, handnum = self.player:getLostHp(), self.player:getHandcardNum()
	if #friends == 0 or x+handnum < 3 or self.player:hasUsed("YjRangliCard") or not self.player:isWounded() then return end
	local card_str = "@YjRangliCard=."
	local card = sgs.Card_Parse(card_str)
	assert(card)
	return card
end

sgs.ai_skill_use_func.YjRangliCard = function(card,use,self)
	use.card = card
	return		
end

sgs.ai_skill_playerchosen.yjrangli = function(self, targets)
	local friends = self.friends_noself 
	self:sort(friends, "handcard")
	return friends[#friends]
end

sgs.ai_skill_invoke.yjcibian = true

sgs.ai_skill_invoke.yjmingmin = true

sgs.ai_skill_cardask["@yjmingmin"] = function(self, data, pattern, target)
	local judge = data:toCard()
	local from = self.room:getTag("MingminTarget"):toPlayer()	
	local cards = self.player:getCards("h")
	for _, cd in sgs.qlist(cards) do
		if cd:objectName() == "peach" or cd:objectName() == "ex_nihilo" then 
			cards:removeOne(cd)
		end
	end
	cards = sgs.QList2Table(cards)
	local id = -1	
	if self:isEnemy(from) then
		for _, cd in ipairs(cards) do
			if cd:getColor() == judge:getColor() and cd:getSuit() == judge:getSuit() then id = cd:getEffectiveId() end
		end
		if id == -1 then
			for _, cd in ipairs(cards) do
				if cd:getColor() == judge:getColor() or cd:getSuit() == judge:getSuit() then				
					id = cd:getEffectiveId()
					break
				end
			end
		end
	elseif self:isFriend(from) then
		if damage.from:getJudgingArea():isEmpty() then
			for _, cd in ipairs(cards) do
				if cd:getColor() == judge:getColor() then id = cd:getEffectiveId() end
			end
		else
			for _, cd in ipairs(cards) do
				if cd:getColor() == judge:getColor() and cd:getSuit() == judge:getSuit() then id = cd:getEffectiveId() end
			end
		end
	end
	if id == -1 then return "."
	else return id
	end
end

sgs.ai_skill_playerchosen.yjpianquan = function(self, targets)
	local friends = self.friends_noself 
	self:sort(friends, "handcard")
	return friends[1]
end

sgs.ai_skill_invoke.yjrenxin = function(self, data)
	local lord = self.room:getLord()
	if self:isFriend(lord) then return true
	else return false
	end
end

sgs.ai_skill_choice.yjrenxin = function(self, choices)
	local target = self.room:getTag("RenxinTarget"):toPlayer()
	if target:getJudgingArea():isEmpty() then return "draw"
	else return "getacard" end
end

local yjyinjian_skill = {}
yjyinjian_skill.name = "yjyinjian"
table.insert(sgs.ai_skills, yjyinjian_skill)
yjyinjian_skill.getTurnUseCard = function(self)
	if self.player:getPile("savage"):isEmpty() then return end
	local card_str = "@YjYinjianCard=."
	local card = sgs.Card_Parse(card_str)
	assert(card)
	return card
end

sgs.ai_skill_use_func.YjYinjianCard = function(card, use, self)
	use.card = card
	local targets = self.room:getAllPlayers()
	local inRange, outRange = {}, {}
	for _, p in sgs.qlist(targets) do
		if self.player:inMyAttackRange(p) then table.insert(inRange, p)
		else table.insert(outRange, p) end
	end	
	self:sort(outRange, "chaofeng")
	if not self.player:hasFlag("out_range") then
		for _, p in ipairs(outRange) do
			if self:isEnemy(p) then 
				if use.to then use.to:append(p) return end
			end
		end
	end
	self:sort(inRange, "handcard")
	for _, p in ipairs(inRange) do
		if self:isFriend(p) then
			if use.to then use.to:append(p) return end
		end
	end
end

sgs.ai_skill_askforag.yjyinjian = function(self, card_ids)
	self.yjyinjian=card_ids[math.random(1,#card_ids)]
	return self.yjyinjian
end

sgs.ai_skill_invoke.yjyinjian = function(self, data)
	local effect = data:toCardEffect()
	if self:isFriend(effect.to) and not self.player:getPile("savage"):isEmpty() then
		return true
	else
		return false
	end
end