sgs.ai_skill_invoke.shuijingh = true

sgs.ai_skill_cardask["@shuijingh"] = function(self)
	local condition = self.room:getTag("Shuijingh"):toString()
	local conditions = condition:split("+")
	local target = self.player
	for _,p in sgs.qlist(self.room:getAllPlayers()) do
		if p:objectName() == conditions[1] then target = p end
	end
	local suit = conditions[2]
	local reason = conditions[3]
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

local mengyunh_skill = {}
mengyunh_skill.name = "mengyunh"
table.insert(sgs.ai_skills, mengyunh_skill)
mengyunh_skill.getTurnUseCard = function(self)
	local cards = self.player:getCards("h")	
	cards = sgs.QList2Table(cards)
	
	local slash
	
	self:sortByUseValue(cards,true)
	
	for _,card in ipairs(cards)  do
		if card:inherits("BasicCard") and not card:inherits("Slash") then
			slash = card
			break
		end
	end
	
	if not slash then return nil end
	local suit = slash:getSuitString()
	local number = slash:getNumberString()
	local card_id = slash:getEffectiveId()
	local card_str = ("slash:mengyunh[%s:%s]=%d"):format(suit, number, card_id)
	local slash_card = sgs.Card_Parse(card_str)
	assert(slash_card)
	
	return slash_card		
end

sgs.ai_view_as.mengyunh = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card_place ~= sgs.Player_Equip then
		if card:inherits("BasicCard") and not card:inherits("Slash") then
			return ("slash:mengyunh[%s:%s]=%d"):format(suit, number, card_id)
		elseif card:inherits("BasicCard") and not card:inherits("Jink") then
			return ("jink:mengyunh[%s:%s]=%d"):format(suit, number, card_id)
		end
	end
end

sgs.ai_skill_cardask["@ziaoh-discard"] = function(self, data)
	local effect = data:toCardEffect()
	if self:isFriend(effect.to) and not
		(effect.to:hasSkill("leiji") and (self:getCardsNum("Jink", effect.to)>0 or (not self:isWeak(effect.to) and self:isEquip("EightDiagram",effect.to))))
		then return "." end
		
	for _, card in sgs.qlist(self.player:getHandcards()) do
		if not card:inherits("Peach") and not card:inherits("ExNihilo") then return "$"..card:getEffectiveId() end 
	end
end

local guijih_skill = {}
guijih_skill.name = "guijih"
table.insert(sgs.ai_skills, guijih_skill)
guijih_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("GuijihCard") then
		return 
	end
	if not self.player:isNude() then
		local card
		local card_id
		if self.player:getHandcardNum() > self.player:getHp() then
			local cards = self.player:getHandcards()
			cards=sgs.QList2Table(cards)
			
			for _, acard in ipairs(cards) do
				if (acard:inherits("BasicCard") or acard:inherits("EquipCard") or acard:inherits("AmazingGrace"))
					and not acard:inherits("Peach") and not acard:inherits("Shit") then 
					if acard:isRed() then 
						card_id = acard:getEffectiveId()
						break
					end
				end
			end
		elseif not self.player:getEquips():isEmpty() then
			local player=self.player
			if player:getWeapon() and player:getWeapon():isRed() then card_id=player:getWeapon():getId()
			elseif player:getOffensiveHorse()and player:getOffensiveHorse():isRed()  then card_id=player:getOffensiveHorse():getId()
			elseif player:getDefensiveHorse()and player:getDefensiveHorse():isRed()  then card_id=player:getDefensiveHorse():getId()
			elseif player:getArmor()and player:getArmor():isRed() and player:getHandcardNum()<=1 then card_id=player:getArmor():getId()
			end
		end
		if not card_id then
			cards=sgs.QList2Table(self.player:getHandcards())
			for _, acard in ipairs(cards) do
				if (acard:inherits("BasicCard") or acard:inherits("EquipCard") or acard:inherits("AmazingGrace"))
					and not acard:inherits("Peach") and not acard:inherits("Shit") then 
					if acard:isRed() then 
						card_id = acard:getEffectiveId()
						break
					end
				end
			end
		end
		if not card_id then
			return nil
		else
			card = sgs.Card_Parse("@GuijihCard=" .. card_id)
			return card
		end
	end
	return nil
end

sgs.ai_skill_use_func.GuijihCard = function(card,use,self)
	if not self.player:hasUsed("GuijihCard") then
		self:sort(self.enemies,"threat")

		for _, friend in ipairs(self.friends_noself) do
			if self:hasSkills(sgs.lose_equip_skill, friend) then

				for _, enemy in ipairs(self.enemies) do
					use.card = card
					
					if use.to then use.to:append(friend) end
					if use.to then use.to:append(enemy) end
					return
				end
			end
		end

		local n = nil
		local final_enemy = nil
		for _, enemy in ipairs(self.enemies) do
			if not self.room:isProhibited(self.player, enemy, card)
				and self:hasTrickEffective(card, enemy)
				and not self:hasSkill(sgs.lose_equip_skill, enemy)
				and not enemy:hasSkill("weimu") then

				for _, enemy2 in ipairs(self.enemies) do
					if enemy:getHandcardNum() == 0 then
						use.card = card
						if use.to then use.to:append(enemy) end
						if use.to then use.to:append(enemy2) end
						return
					else
						n = 1;
						final_enemy = enemy2
					end
				end
				if n then use.card = card end
				if use.to then use.to:append(enemy) end
				if use.to then use.to:append(final_enemy) end
				return

			end
			n = nil
		end
	end
end

sgs.ai_use_value.GuijihCard = 8.5
sgs.ai_use_priority.GuijihCard = 4

guijih_filter = function(player, carduse)
	if carduse.card:inherits("GuijihCard") then
		sgs.ai_guijih_effect = true
	end
end

table.insert(sgs.ai_choicemade_filter.cardUsed, guijih_filter)

sgs.ai_card_intention.GuijihCard = function(card, from, to)
	if sgs.evaluateRoleTrends(to[1]) == sgs.evaluateRoleTrends(to[2]) then
		sgs.updateIntentions(from, to, 40)
	end
end

sgs.dynamic_value.damage_card.GuijihCard = true

sgs.ai_skill_invoke.baohenh = function(self, data)
	local ces = data:toCardEffect()
	if self:isFriend(ces.to) then
		return ces.to:isChained()
	else
		return not ces.to:isChained()
	end
end

sgs.ai_skill_invoke.weicongh = function(self, data)
	local xuji = data:toPlayer()
	if not self:isWeak(xuji) then
		return true
	else
		return false
	end
end

sgs.ai_skill_invoke.xiurenh = function(self, data)
	local damage = data:toDamage()
	local players = self.enemies
	if self:isEnemy(damage.to) and #players > 1 and not self:isWeak(damage.to) then
		return true
	else
		return false
	end
end

sgs.ai_skill_choice.xiurenh = function(self, choices)
	if not self.player:isWounded() then return "dra2cd" 
	elseif self.player:hasSkill("jijiu") then return "dra2cd"
	else return "rec1hp" end	
end