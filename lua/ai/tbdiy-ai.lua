--xuelu
sgs.ai_skill_invoke.diyxuelu = function(self, data)
	local effect = data:toSlashEffect()
	if self:isFriend(effect.from) then return false end
	
	if self.player:getHp() > 1 then
		if effect.to:objectName() == self.player:objectName() and self:getCardsNum("Jink") == 0 then return true end
		
		self:sort(self.enemies, "hp")
		
		for _, enemy in ipairs(self.enemies) do
			if enemy:getHp() == 1 and self:getCardsNum("Slash") == 0 and self.player:canSlash(enemy) then
				return true
			end
		end
		
		if self:isFriend(effect.to) then
			if math.abs(self.player:getHp() - effect.to:getHp()) > 1 then 
				return true 
			elseif self.player:getHp() > 1 and effect.to:getHp() == 1 then 
				return true
			end 
		else
			return false
		end
	else
		return false
	end
end

sgs.ai_skill_cardask["@diyxuelu-horse"] = function(self, data, pattern, target)

	local to_discard
	local cards = self.player:getCards("he")
	local has_defenhorse = false
	local has_offenhorse = false
	
	for _, card in sgs.qlist(cards) do
		if card:inherits("OffensiveHorse") and not self.player:isJilei(card) then
			has_offenhorse = true
		elseif card:inherits("DefensiveHorse") and not self.player:isJilei(card) then
			has_defenhorse = true
		end
	end
	
	if self.player:getHp() >= 2 then
		if has_offenhorse then 
			to_discard = self:getCardId("OffensiveHorse")
		else 
			return "."
		end
	else
		if has_offenhorse then
			to_discard = self:getCardId("OffensiveHorse")
		elseif has_defenhorse then
			to_discard = self:getCardId("DefensiveHorse")
		else
			return "."
		end
	end
	
	if to_discard ~= nil then
		return to_discard
	end
end

--shenzhi
local diyshenzhi_skill = {}
diyshenzhi_skill.name = "diyshenzhi"
table.insert(sgs.ai_skills, diyshenzhi_skill)
diyshenzhi_skill.getTurnUseCard = function(self)
	if self.player:getHandcardNum() > self.player:getHp() and self:getCardsNum("Jink") > 1 then
		if (not self.player:hasUsed("Slash")) or (self.player:getWeapon() and self.player:getWeapon():objectName() == "Cross" and self:getCardsNum("Slash") > 1) then return nil end
	end
	local cards = self.player:getHandcards()
	cards=sgs.QList2Table(cards)
	
	local card = nil
	self:sortByUseValue(cards,true)
	for _, card in ipairs(cards) do
		if not card:inherits("Peach") then
			card = cards[1]
		end
	end	

	if (not card) or self.player:hasUsed("DiyShenZhiCard") then return nil end
	
	local card_id = card:getEffectiveId()
	local card_str = ("@DiyShenZhiCard=%d"):format(card_id)
	local skillcard = sgs.Card_Parse(card_str)
	assert(skillcard)
	return skillcard
end

sgs.ai_skill_use_func.DiyShenZhiCard = function(card, use, self)
	self:sort(self.enemies, "hp")
	local target 
	for _, enemy in ipairs(self.enemies) do
		if not enemy:isKongcheng() then
			target = enemy
			break
		end
	end
	use.card = card
	if target and use.to then use.to:append(target) end
	return	
end

sgs.ai_skill_choice.diyshenzhi = function(self, choices)
	if self:getCardsNum("Peach") > 0 or self.player:getHp() > 1 then return "obtain" end
	return "reject"
end

--zhaolie
sgs.ai_skill_invoke.diyzhaolie = true

--doudan
--[[
sgs.ai_skill_use["@@diydoudan"] = function(self, prompt)
	if (self.player:getHp() > 2 and self.player:getHandcardNum() > 2) or self.player:getNextAlive():getCards("j"):contains("Lightning") or (self.player:getHp() <= 1 and (self:getCardsNum("Peach") < 1 or self:getCardsNum("Analeptic") < 1)) then
		return ("@DiyDouDanCard=.->.")
	end
end

sgs.ai_skill_choice.diydoudan = function(self, choices)
	local cdids = room:getTag("DouDanCards"):toIntList()
	local has_Jink, has_Nullification, has_Peach, has_Analeptic, has_Slash = false, false, false, false, false
	for _, id in sgs.qlist(cdids) do
		local card = sgs.Sanguosha:getCard(id)
		if card:inherits("Jink") then has_Jink = true end
		if card:inherits("Nullification") then has_Nullification = true end
		if card:inherits("Peach") then has_Peach = true end
		if card:inherits("Analeptic") then has_Analeptic = true end
		if card:inherits("Slash") then has_Slash = true end
	end
	if (self.player:getHp() > 2 and self.player:getHandcardNum() > 2) and (has_Jink or has_Nullification) then return "obtain1cd" end
	if self.player:getNextAlive():getCards("j"):contains("Lightning") then return "guan3xing" end
	if (self.player:getHp() <= 1 and (self:getCardsNum("Peach") < 1 or self:getCardsNum("Analeptic") < 1)) and (has_Peach or has_Analeptic) then return "obtain1cd" end
	return "guan3xing"
end

sgs.ai_skill_askforag.diydoudan = function(self, card_ids)
	local has_Jink, has_Nullification, has_Peach, has_Analeptic, has_Slash = false, false, false, false, false
	local jinkid, nullificationid, peachid, analepitcid, slashid
	for _, id in sgs.qlist(card_ids) do
		local card = sgs.Sanguosha:getCard(id)
		if card:inherits("Jink") then 
			has_Jink = true 
			jinkid = id
		end
		if card:inherits("Nullification") then 
			has_Nullification = true 
			nullificationid = id
		end
		if card:inherits("Peach") then 
			has_Peach = true 
			peachid = id
		end
		if card:inherits("Analeptic") then 
			has_Analeptic = true 
			analepticid = id
		end
		if card:inherits("Slash") then 
			has_Slash = true 
			slashid = id
		end
	end
	
	if has_Peach then
		self.diydoudan = peachid
	elseif has_Analepitc then
		self.diydoudan = analepitcid
	elseif has_Jink then
		self.diydoudan = jinkid
	elseif has_Nullification then
		self.diydoudan = nullificationid
	elseif has_Slash then
		self.diydoudan = slashid
	end
	
	return self.diydoudan
end
]]
--mingzhi
sgs.ai_skill_invoke.diymingzhi = true

--chishen
sgs.ai_skill_invoke.diychishen = true