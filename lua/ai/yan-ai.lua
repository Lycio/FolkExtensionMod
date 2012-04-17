sgs.ai_view_as.yanyanhu = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card:getSuitString() == "heart" and not card:inherits("Peach") then
		return ("cover:yanyanhu[%s:%s]=%d"):format(suit, number, card_id)
	end
end

sgs.ai_skill_invoke.yanfanji = function(self, data)
	if self.player:isKongcheng() then return false end
	return true
end

sgs.ai_skill_choice.yanfanji = function(self, choices)
	if self.player:isWounded() and self:getCardsNum("Jink") > 0 then
		return "recoverhp"
	else
		return "drawcards"
	end
end

function sgs.ai_skill_pindian.yanfanji(minusecard, self, requestor)
	if self:isFriend(requestor) then return end
	if requestor:getHandcardNum() <= 2 then return minusecard end
end

sgs.ai_cardneed.yanfanji = sgs.ai_cardneed.bignumber

sgs.ai_view_as.yanxianfeng = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card:getSuitString() == "heart" and not card:inherits("Peach") and card_place == sgs.Player_Hand then
		return ("sudden_strike:yanxianfeng[%s:%s]=%d"):format(suit, number, card_id)
	end
end

sgs.ai_skill_invoke.yanbaoshang = true

sgs.ai_skill_invoke.yanjiaozhan = function(self, data)
	if self:getCardsNum("Slash") >= 1 then
		for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if self:isWeak(p) and not self:isFriend(p) then
				return true
			end
		end
	end
	return false
end

sgs.ai_skill_playerchosen.yanjiaozhan = function(self, targets)
	local targetlist=sgs.QList2Table(targets)
	local target
	for _, player in ipairs(targetlist) do
		if self:isEnemy(player) and (not target or target:getHp() < player:getHp()) or (player:isKongcheng()) then
			target = player
		end
	end
	if target then return target end
	self:sort(targetlist, "hp")
	if self.player:getRole() == "loyalist" and targetlist[1]:isLord() then return targetlist[2] end
	return targetlist[1]
end

local yanjiusha_skill = {}
yanjiusha_skill.name = "yanjiusha"
table.insert(sgs.ai_skills, yanjiusha_skill)
yanjiusha_skill.getTurnUseCard = function(self)
	local handcards = self.player:getCards("h")
	local ids = sgs.IntList()
	for _,cd in sgs.qlist(handcards) do
		if cd:inherits("Jink") and cd:getSuitString() == "heart" then
			ids:append(cd:getEffectiveId())
		end
	end
	if ids:isEmpty() or (not self.player:isWounded()) then return nil end

	local card = sgs.Sanguosha:getCard(ids:first())
	if not card then return nil end
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("peach:yanjiusha[%s:%s]=%d"):format(suit, number, card_id)
	local skillcard = sgs.Card_Parse(card_str)
	assert(skillcard)
	return skillcard
end

sgs.ai_view_as.yanjiusha = function(card, player, card_place, class_name)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card:inherits("Jink") and card:getSuit() == sgs.Card_Heart then
		if class_name == "Peach" then
			return ("peach:yanjiusha[%s:%s]=%d"):format(suit, number, card_id)
		elseif class_name == "PoisonPeach" then
			return ("poison_peach:yanjiusha[%s:%s]=%d"):format(suit, number, card_id)
		end
	end
end

sgs.ai_skill_invoke.yanshenyi = true

sgs.ai_view_as.yanjiefu = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card:isBlack() and card_place == sgs.Player_Hand  then
		return ("rob:yanjiefu[%s:%s]=%d"):format(suit, number, card_id)
	end
end

local yanjiuse_skill = {}
yanjiuse_skill.name = "yanjiuse"
table.insert(sgs.ai_skills,yanjiuse_skill)
yanjiuse_skill.getTurnUseCard = function(self)
	local handcards = self.player:getCards("h")
	local ids = sgs.IntList()
	for _,cd in sgs.qlist(handcards) do
		if cd:inherits("Slash") then
			ids:append(cd:getEffectiveId())
		end
	end
	if ids:isEmpty() then return nil end

	if not self.player:hasUsed("YanJiuseCard") and self:getCardsNum("Slash") > 1 then
		local card = sgs.Sanguosha:getCard(ids:first())
		return sgs.Card_Parse("@YanJiuseCard=" .. card:getEffectiveId())
	end
end

sgs.ai_skill_use_func.YanJiuseCard = function(card,use,self)
	
	if not self.player:hasUsed("YanJiuseCard") then
		self:sort(self.enemies,"threat")

	for _, friend in ipairs(self.friends_noself) do
		if friend:getWeapon() and self:hasSkills(sgs.lose_equip_skill, friend) then

			for _, enemy in ipairs(self.enemies) do
				if friend:canSlash(enemy) then
					use.card = card
				end
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
			and not enemy:hasSkill("weimu")
			and enemy:getWeapon() then

			for _, enemy2 in ipairs(self.enemies) do
				if enemy:canSlash(enemy2) then
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

sgs.ai_use_value.YanJiuseCard = 8.5
sgs.ai_use_priority.YanJiuseCard = 4

yanjiuse_filter = function(player, carduse)
	if carduse.card:inherits("YanJiuseCard") then
		sgs.ai_yanjiuse_effect = true
	end
end

table.insert(sgs.ai_choicemade_filter.cardUsed, yanjiuse_filter)

sgs.ai_skill_invoke.yanmanwu = true

sgs.ai_view_as.yantianhui = function(card, player, card_place, class_name)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if player:getPhase()==sgs.Player_NotActive then
		if card:isBlack() then
			return ("analeptic:yantianhui[%s:%s]=%d"):format(suit, number, card_id)
		elseif card:isRed() then
			return ("peach:yantianhui[%s:%s]=%d"):format(suit, number, card_id)
		end
	end
end

sgs.ai_skill_playerchosen.yanjifeng = function(self, targets)
	local target
	self:sort(self.enemies, "hp")
	target = self.enemies[#self.enemies]
	return target
end

local yancangshan_skill = {}
yancangshan_skill.name = "yancangshan"
table.insert(sgs.ai_skills, yancangshan_skill)
yancangshan_skill.getTurnUseCard = function(self)
	local shans = self.player:getPile("shanpile")
	if shans:isEmpty() or (not self.player:isWounded()) then return nil end

	local card = sgs.Sanguosha:getCard(shans:first())
	if not card then return nil end
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("peach:yancangshan[%s:%s]=%d"):format(suit, number, card_id)
	local skillcard = sgs.Card_Parse(card_str)
	assert(skillcard)
	return skillcard
end

sgs.ai_view_as.yancangshan = function(card, player, card_place)
	local shans = player:getPile("shanpile")
	if shans:isEmpty() then return nil end
	
	local card = sgs.Sanguosha:getCard(shans:first())
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	return ("peach:yancangshan[%s:%s]=%d"):format(suit, number, card_id)
end

sgs.ai_skill_invoke.yanfenshang = function(self, data)
	local invoke = false
	for _, player in sgs.qlist(self.room:getAllPlayers()) do
		if (self:isFriend(player)) and ((not player:containsTrick("indulgence")) or player:hasSkill("keji")) then
			invoke = true
		end
	end
	return invoke
end

sgs.ai_skill_playerchosen.yanfenshang = function(self, targets)
	local target
	for _, player in sgs.qlist(targets) do
		if (self:isFriend(player)) and ((not player:containsTrick("indulgence")) or player:hasSkill("keji")) then
			target = player
		end
	end
	return target
end

sgs.ai_skill_invoke.yanjiuzi = false

sgs.ai_skill_cardask["@yanlongtai"] = function(self, data, pattern, target)
	local effect = data:toCardEffect()
	local cd = effect.card
	local invoke, cd_good = false, false
	if cd:inherits("ExNihilo") or cd:inherits("AmazingGrace") or cd:inherits("GodSalvation") then cd_good = true end
	if self:isFriend(effect.to) then
		if cd_good == true then 
			invoke = false 
		else
			invoke = true
		end
	else
		if cd_good == true then
			invoke = true
		else
			invoke = false
		end
	end
	if invoke == false then return "." end
	
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)
	
	for _, card in ipairs(cards) do
		if card:isRed() then 
			return card:toString()
		end 
	end
	return "."
end

sgs.ai_skill_invoke.yanhuixuan = function(self, data)
	return self.player:getHandcardNum() > self.player:getHp()
end

sgs.ai_skill_cardask["@yanhuixuan"] = function(self, data, pattern, target)
	local effect = data:toCardEffect()
	local target = effect.to
	if self:isFriend(target) or target:objectName() == self.player:objectName() then return "." end
		
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)
	
	for _, card in ipairs(cards) do
		if card:isRed() and not card:inherits("Peach") then 
			return card:toString()
		end 
	end
	return "."
end

sgs.ai_skill_playerchosen.yandaohun = function(self, targets)
	local target
	for _, player in sgs.qlist(targets) do
		if (self:isFriend(player)) then
			target = player
		end
	end
	return target
end

sgs.ai_skill_invoke.yanlogyin = function(self, data)
	local damage = data:toDamage()
	return self:isEnemy(damage.to)
end

sgs.ai_skill_cardask["@guiling-slash"] = function(self, data, pattern, target)
	if target and self:isFriend(target) and not (target:hasSkill("leiji") and self:getCardsNum("Jink", target, "h") > 0) then
		return "."
	end
	for _, slash in ipairs(self:getCards("Slash")) do
		if self:slashIsEffective(slash, target) then 
			return slash:toString()
		end 
	end
	return "."
end

sgs.ai_skill_playerchosen.yanbaiming = function(self, targets)
	local tos = {}
	for _, player in sgs.qlist(targets) do
		if self:isEnemy(player) then table.insert(tos, player) end
	end
	if #tos ~= 0 then
		self:sort(tos, "hp")
		return tos[1]
	else
		self:sort(targets, "hp")
		return targets[#targets]
	end
end

local yanjunling_skill = {}
yanjunling_skill.name = "yanjunling"
table.insert(sgs.ai_skills, yanjunling_skill)
yanjunling_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("YanJunlingCard") then return end
	local card_str = "@YanJunlingCard=."
	local card = sgs.Card_Parse(card_str)
	assert(card)

	return card
end

sgs.ai_skill_use_func.YanJunlingCard = function(card, use, self)
	local index1, index2 = nil, nil
	local players = self.room:getAllPlayers()
	if players:length() == 2 then
		index1 = self.room:getOtherPlayers(self.player):first()
		index2 = self.player
	elseif players:length() > 2 then
		local enemies = self.enemies
		if #enemies == 1 then
			for _, player in sgs.qlist(self.friends_noself) do
				local slash_num = 0
				for _, cd in sgs.qlist(player:getCards("h")) do
					if cd:inherits("Slash") then slash_num = slash_num + 1 end
				end
				if player:canSlash(enemies[1]) and slash_num > 0 then
					index1 = player
					index2 = enemies[1]
					break
				end
			end
			if index1 == nil then
				index1 = enemies[1]
				local friends = self.friends_noself
				self:sort(friends, "defense")
				index2 = friends[#friends]
			end
		elseif #enemies > 1 then
			self:sort(self.enemies, "chaofeng")
			for _, enemy in ipairs(enemies) do
				for _, to in ipairs(enemies) do
					if to:objectName() ~= enemy:objectName() and enemy:canSlash(to) then
						index1 = enemy
						index2 = to
						break
					end
				end
			end
			if index1 == nil then
				for _, friend in ipairs(self.friends_noself) do
					for _, enemy in ipairs(enemies) do
						if friend:canSlash(enemies) then
							index1 = friend
							index2 = enemy
							break
						end
					end
				end
			end
		end
	end	
	if index1 and index2 and index1:objectName() ~= index2:objectName() then
		use.card = card
		if use.to then
			use.to:append(index1)
			use.to:append(index2)
		end
	end
end