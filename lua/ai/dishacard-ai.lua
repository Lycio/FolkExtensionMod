--keep_value
sgs.ai_keep_value.PunctureSlash = 2.2
sgs.ai_keep_value.BloodSlash = 2.4
sgs.ai_keep_value.PoisonPeach = 5
sgs.ai_keep_value.Plague = -1
sgs.ai_keep_value.SuddenStrike = 3
sgs.ai_keep_value.Cover = 4.5
sgs.ai_keep_value.IrresistibleForce = 2
sgs.ai_keep_value.Rebound = 4.5
sgs.ai_keep_value.Rob = 3.5

--use_value
sgs.ai_use_value.PunctureSlash = 4.4
sgs.ai_use_value.BloodSlash = 4.4
sgs.ai_use_value.PoisonPeach = 2
sgs.ai_use_value.Plague = -1
sgs.ai_use_value.SuddenStrike = 3
sgs.ai_use_value.Cover = 6
sgs.ai_use_value.IrresistibleForce = 2
sgs.ai_use_value.Rebound = 6
sgs.ai_keep_value.Rob = 5.5

sgs.weapon_range.YitianJian = 2
sgs.weapon_range.QixingBlade = 2
sgs.weapon_range.JiaSuo = 1
sgs.weapon_range.LuofengBow = 1

function SmartAI:slashIsEffective(slash, to)
	if (to:hasSkill("yinshih") and not to:canSlash(self.player)) or (self.player:hasSkill("yinshih") and not to:canSlash(self.player)) then return false end
    if to:hasSkill("zuixiang") and to:isLocked(slash) then return false end
	if to:hasSkill("heiyan") then 
		if slash:isBlack() then return false end
	end
	if to:hasSkill("yizhong") and not to:getArmor() then
		if slash:isBlack() then
			return false
		end
	end

	local natures = {
		Slash = sgs.DamageStruct_Normal,
		BloodSlash = sgs.DamageStruct_Normal,
		PunctureSlash = sgs.DamageStruct_Normal,
		FireSlash = sgs.DamageStruct_Fire,
		ThunderSlash = sgs.DamageStruct_Thunder,
	}

	local nature = natures[slash:className()]
	if self.player:hasSkill("zonghuo") then nature = sgs.DamageStruct_Fire end
	if not self:damageIsEffective(to, nature) then return false end

	if self.player:hasWeapon("qinggang_sword") or (self.player:hasFlag("xianzhen_success") and self.room:getTag("XianzhenTarget"):toPlayer() == to) then
		return true
	end

	local armor = to:getArmor()
	if armor then
		if armor:objectName() == "renwang_shield" then
			return not slash:isBlack()
		elseif armor:objectName() == "vine" then
			return nature ~= sgs.DamageStruct_Normal or self.player:hasWeapon("fan")
		end
	end

	return true
end

local function hasExplicitRebel(room)
	for _, player in sgs.qlist(room:getAllPlayers()) do
		if sgs.isRolePredictable() and  sgs.evaluatePlayerRole(player) == "rebel" then return true end
		if sgs.compareRoleEvaluation(player, "rebel", "loyalist") == "rebel" then return true end
	end
	return false
end

function SmartAI:useCardSlash(card, use)
	if not self:slashIsAvailable() then return end
	local basicnum = 0
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)
	for _, acard in ipairs(cards) do
		if acard:getTypeId() == sgs.Card_Basic and not acard:inherits("Peach") then basicnum = basicnum + 1 end
	end
	local no_distance = self.slash_distance_limit
	if card:getSkillName() == "wushen" then no_distance = true end
	if card:getSkillName() == "gongqi" then no_distance = true end
	if card:getSkillName() == "lihuo" or (self.player:hasSkill("lihuo") and card:inherits("FireSlash")) then self.slash_targets = 2 end
	if self.player:hasSkill("longyin") and card:isBlack() then no_distance = true end
	if self.player:hasSkill("huxiao") and card:isRed() then self.slash_targets = 2 end
	if self.player:hasSkill("juelu") and self.player:getHandcardNum() == 1 then 
		no_distance = true
		self.slash_targets = 2
	end
	if (self.player:getHandcardNum() == 1
	and self.player:getHandcards():first():inherits("Slash")
	and self.player:getWeapon()
	and self.player:getWeapon():inherits("Halberd"))
	or (self.player:hasSkill("shenji") and not self.player:getWeapon()) then
		self.slash_targets = 3
	end

	self.predictedRange = self.player:getAttackRange()
	if self.player:hasSkill("qingnang") and self:isWeak() and self:getOverflow() == 0 then return end
	local huatuo = self.room:findPlayerBySkillName("jijiu")
	for _, friend in ipairs(self.friends_noself) do
		local slash_prohibit = false
		slash_prohibit = self:slashProhibit(card,friend)
		if (self.player:hasSkill("pojun") and friend:getHp() > 4 and self:getCardsNum("Jink", friend) == 0
			and friend:getHandcardNum() < 3)
		or (friend:hasSkill("leiji") 
		and (self:getCardsNum("Jink", friend) > 0 or (not self:isWeak(friend) and self:isEquip("EightDiagram",friend)))
		and (hasExplicitRebel(self.room) or not friend:isLord()))
		or (friend:isLord() and self.player:hasSkill("guagu") and friend:getLostHp() >= 1 and self:getCardsNum("Jink", friend) == 0)
		or (friend:hasSkill("jieming") and self.player:hasSkill("rende") and (huatuo and self:isFriend(huatuo)))
		then
			if not slash_prohibit then
				if ((self.player:canSlash(friend, not no_distance)) or
					(use.isDummy and (self.player:distanceTo(friend) <= self.predictedRange))) and
					self:slashIsEffective(card, friend) then
					use.card = card
					if use.to then
						use.to:append(friend)
						self:speak("hostile", self.player:getGeneral():isFemale())
						if self.slash_targets <= use.to:length() then return end
					end
				end
			end
		end
	end

	local targets = {}
	local ptarget = self:getPriorTarget()
	if ptarget and not self:slashProhibit(card, ptarget) then 
		table.insert(targets, ptarget)
	end
	self:sort(self.enemies, "defense")
	for _, enemy in ipairs(self.enemies) do
		local slash_prohibit = false
		slash_prohibit = self:slashProhibit(card,enemy)
		if not slash_prohibit and enemy:objectName() ~= ptarget:objectName() then 
			table.insert(targets, enemy)
		end
	end
	
	for _, target in ipairs(targets) do
		local canliuli = false
		for _, friend in ipairs(self.friends_noself) do
			if self:canLiuli(target, friend) and self:slashIsEffective(card, friend) then canliuli = true end
		end
		if (self.player:canSlash(target, not no_distance) or
		(use.isDummy and self.predictedRange and (self.player:distanceTo(target) <= self.predictedRange))) and
		self:objectiveLevel(target) > 3
		and self:slashIsEffective(card, target) and
		not (target:hasSkill("xiangle") and basicnum < 2) and not canliuli and
		not (not self:isWeak(target) and #self.enemies > 1 and #self.friends > 1 and self.player:hasSkill("keji")
			and self:getOverflow() > 0 and not self:isEquip("Crossbow")) then
			-- fill the card use struct
			local usecard = card
			if not use.to or use.to:isEmpty() then
				local anal = self:searchForAnaleptic(use,target,card)
				if anal and not self:isEquip("SilverLion", target) and not self:isWeak() then
					if anal:getEffectiveId() ~= card:getEffectiveId() then use.card = anal return end
				end
				local equips = self:getCards("EquipCard", self.player, "h")
				for _, equip in ipairs(equips) do
					local callback = sgs.ai_slash_weaponfilter[equip:objectName()]
					if callback and type(callback) == "function" and callback(target, self) and
						self.player:distanceTo(target) <= (sgs.weapon_range[equip:className()] or 0) then
						self:useEquipCard(equip, use)
						if use.card then return end
					end
				end
				if target:isChained() and self:isGoodChainTarget(target) and not use.card then
					if self:isEquip("Crossbow") and card:inherits("NatureSlash") then
						local slashes = self:getCards("Slash")
						for _, slash in ipairs(slashes) do
							if not slash:inherits("NatureSlash") and self:slashIsEffective(slash, target)
								and not self:slashProhibit(slash, target) then
								usecard = slash
								break
							end
						end
					elseif not card:inherits("NatureSlash") then
						local slash = self:getCard("NatureSlash")
						if slash and self:slashIsEffective(slash, target) and not self:slashProhibit(slash, target) then usecard = slash end
					end
				end
			end
			use.card = use.card or usecard
			if use.to and not use.to:contains(target) then 
				use.to:append(target) 
				if self.slash_targets <= use.to:length() then return end
			end
		end 
	end

	for _, friend in ipairs(self.friends_noself) do
		if friend:hasSkill("yiji") and friend:getLostHp() < 1 and
			not (friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage")) then
			local slash_prohibit = false
			slash_prohibit = self:slashProhibit(card, friend)
			if not slash_prohibit then
				if ((self.player:canSlash(friend, not no_distance)) or
					(use.isDummy and (self.player:distanceTo(friend) <= self.predictedRange))) and
					self:slashIsEffective(card, friend) then
					use.card = card
					if use.to then
						use.to:append(friend)
						self:speak("yiji")
						if self.slash_targets <= use.to:length() then return end
					end
				end
			end
		end
	end
end

function SmartAI:askForSinglePeach(dying)
	local card_str
	local forbid = sgs.Sanguosha:cloneCard("peach", sgs.Card_NoSuit, 0)
	if self.player:isLocked(forbid) or dying:isLocked(forbid) then return "." end
	
	if (dying:hasSkill("yinshih") and not dying:canSlash(self.player)) or (self.player:hasSkill("yinshih") and not dying:canSlash(self.player)) then return "." end
	
	if self:isFriend(dying) then
		if self:needDeath(dying) then return "." end
		local buqu = dying:getPile("buqu")
		local weaklord = false
		if not buqu:isEmpty() then
			local same = false
			for i, card_id in sgs.qlist(buqu) do
				for j, card_id2 in sgs.qlist(buqu) do
					if i ~= j and sgs.Sanguosha:getCard(card_id):getNumber() == sgs.Sanguosha:getCard(card_id2):getNumber() then
						same = true
						break
					end
				end
			end
			if not same then return "." end
		end
		if (self.player:objectName() == dying:objectName()) then
			card_str = self:getCardId("Analeptic") or self:getCardId("Peach")
		elseif dying:isLord() then
			card_str = self:getCardId("Peach")
		else
			for _, friend in ipairs(self.friends_noself) do
				if friend:getHp() == 1 and friend:isLord() and not friend:hasSkill("buqu") then  weaklord = true end
			end
			for _, enemy in ipairs(self.enemies) do
				if enemy:getHp() == 1 and enemy:isLord() and not enemy:hasSkill("buqu") and self.player:getRole() == "renegade" then weaklord = true end
			end
			if not weaklord or self:getAllPeachNum() > 1 then
				card_str = self:getCardId("Peach") 
			end
		end
	elseif not self:isFriend(dying) and self:objectiveLevel(dying) > 3 and 1 - dying:getHp() <= 1 then
		card_str = self:getCardId("PoisonPeach")
	end
	return card_str or "."
end

function SmartAI:useCardPunctureSlash(...)
	self:useCardSlash(...)
end

function SmartAI:useCardBloodSlash(...)
	self:useCardSlash(...)
end

function sgs.ai_slash_weaponfilter.luofeng_bow(to, self)
	return to:getHandcardNum() > self.player:getHp()
end

sgs.ai_skill_cardask["@yitian_jian"] = function(self, data, pattern, target)
	local slashs = {}
	for _, cd in sgs.qlist(self.player:getCards("h")) do
		if cd:inherits("Slash") then 
			table.insert(slashs, cd)
		end
	end
	if #slashs < 2 then return "." end
	
	return "$"..slashs[1]:getEffectiveId().."+"..slashs[2]:getEffectiveId()
end

sgs.ai_skill_choice.yitian_jian = function(self, choices)
	local target = self.room:getTag("YitianjianTarget"):toPlayer()
	if self:isFriend(target) then
		return "torecover"
	else
		return "todamage"
	end
end

function SmartAI:useCardJiaoLiao(card, use)
	use.broken = true
	for _, enemy in ipairs(self.enemies) do
		if not enemy:getArmor() and not self:hasSkills("jijiu|leiji|longhun",enemy) then
			use.card = card
			if use.to then
				use.to:append(enemy)
			end
			return
		end
	end
end

sgs.ai_skill_cardask["@jiaoliao-jink-1"] = function(self, data, pattern, target)
	if sgs.ai_skill_cardask.nullfilter(self, data, pattern, target) then return "." end
	if self:getCardsNum("Jink") < 2 and not (self.player:getHandcardNum() == 1 and self:hasSkills(sgs.need_kongcheng)) then return "." end	
end

function sgs.ai_weapon_value.YitianJian(self, enemy)
	return 2 
end

function sgs.ai_weapon_value.QixingBlade(self, enemy)
	return 2
end

function sgs.ai_weapon_value.LuofengBow(self, enemy)
	if enemy and enemy:getHandcardNum() > self.player:getHp() then return 10 end
end

function sgs.ai_weapon_value.JiaSuo(self, enemy)
	return -10
end

sgs.ai_armor_value["jiaoliao"] = function()
	return -10
end

function SmartAI:useCardJiaSuo(card, use)
	use.broken = true
	for _, enemy in ipairs(self.enemies) do
		use.card = card
		if use.to then
			use.to:append(enemy)
		end
		return
	end
end

function SmartAI:useCardIrresistibleForce(card, use)
	for _, enemy in ipairs(self.enemies) do
		if enemy:getEquips():length() > 0 then
			use.card = card
			return
		end
	end
end

function sgs.ai_slash_prohibit.leiji(self, to, card)
	if self:isFriend(to) then return false end
	if to:getArmor() and to:getArmor():inherits("JiaoLiao") then return false end	
	local hcard = to:getHandcardNum()
	if self.player:hasSkill("tieji") or
		(self.player:hasSkill("liegong") and (hcard>=self.player:getHp() or hcard<=self.player:getAttackRange())) then return false end
	if to:getHandcardNum() >= 2 then return true end
	if self:isEquip("EightDiagram", to) then
		local equips = to:getEquips()
		for _, equip in sgs.qlist(equips) do
			if equip:getSuitString() == "spade" then return true end
		end
	end
end

function SmartAI:askForNullification(trick, from, to, positive)
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local null_card
	null_card = self:getCardId("Nullification")
	if null_card then null_card = sgs.Card_Parse(null_card) else return end

	if positive then
		if from and self:isEnemy(from) and (sgs.evaluateRoleTrends(from) ~= "neutral" or sgs.isRolePredictable()) then
			if trick:inherits("ExNihilo") and self:getOverflow(from) == 0 then return null_card end
			if trick:inherits("IronChain") and not self:isEquip("Vine", to) then return nil end
			if self:isFriend(to) then
				if trick:inherits("Dismantlement") then
					if to:getArmor() then return null_card end
				else
					if trick:inherits("Snatch") then return null_card end
					if trick:inherits("Rob") then return null_card	end
					if trick:inherits("Rebound") then return null_card end
					if trick:inherits("Cover") then return null_card end
					if self:isWeak(to) then
						if trick:inherits("Duel") then
							return null_card
						elseif trick:inherits("FireAttack") then
							if from:getHandcardNum() > 2 then return null_card end
						end
					end
				end
			elseif self:isEnemy(to) then
				if (trick:inherits("Snatch") or trick:inherits("Dismantlement")) and to:getCards("j"):length() > 0 then
					return null_card
				end
			end
		end

		if self:isFriend(to) then
			if trick:inherits("Indulgence") or trick:inherits("SupplyShortage") then
				return null_card
			end
			if self:isWeak(to) then
				if trick:inherits("ArcheryAttack") then
					if self:getCardsNum("Jink", to) == 0 then return null_card end
				elseif trick:inherits("SavageAssault") then
					if self:getCardsNum("Slash", to) == 0 then return null_card end
				elseif trick:inherits("SuddenStrike") then
					if self:getCardsNum("Jink", to) == 0 then return null_card end
				elseif trick:inherits("IrresistibleForce") then
					if to:getEquips():length() > 0 then return null_card end
				end
			end
		end
		if from then
			if self:isEnemy(to) then
				if trick:inherits("GodSalvation") and self:isWeak(to) then
					return null_card
				end
			end
		end
	else
		if from then
			if from:objectName() == to:objectName() then
				if self:isFriend(from) then return null_card else return end
			end
			if not (trick:inherits("AmazingGrace") or trick:inherits("GodSalvation") or trick:inherits("AOE")) then
				if self:isFriend(from) then
					if ("snatch|dismantlement"):match(trick:objectName()) and to:isNude() then
					elseif trick:inherits("FireAttack") and to:isKongcheng() then
					else return null_card end
				end
			end
		else
			if self:isEnemy(to) and (sgs.evaluateRoleTrends(to) ~= "neutral" or sgs.isRolePredictable()) then return null_card else return end
		end
	end
end

function SmartAI:askForCover(effect)
	local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local card
	card = self:getCardId("Cover")
	if card then card = sgs.Card_Parse(card) else return end
	
	if effect.card:inherits("Duel") and self.player ~= effect.to then
		if(self.player ~= effect.to and self:isFriend(effect.to)) then
			if(self:getCardsNum("Slash") > 0 or self:getCardsNum("Rebound") > 0)then
				return card
			end
		elseif(self.player:hasSkill("tianxiang"))then
			return card
		end
	elseif effect.card:inherits("Slash") and self.player ~= effect.to then
		if(self:isFriend(effect.to)) then
			if(effect.to:getArmor() and effect.to:getArmor():objectName() == "eight_diagram")then return end
			if(self.player:getArmor() and (self.player:getArmor():objectName() == "jiaoliao" or self.player:getArmor():objectName() == "vine"))then return end
			if(self:getCardsNum("Jink") > 0 or self:getCardsNum("Rebound") > 0)then
				return card
			end
		elseif(self.player:hasSkill("leiji") and self:getCardsNum("Jink") > 0)then
			return card
		end
    end
end

function SmartAI:askForRebound(damage)
	local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local card
	card = self:getCardId("Rebound")
	if card then card = sgs.Card_Parse(card) else return end
	
	if damage.from and self.player:objectName() ~= damage.from:objectName() and not self:isFriend(damage.from) then
		return card
	end
end

function SmartAI:askForRob(damage)
	local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local card
	card = self:getCardId("Rob")
	if card then card = sgs.Card_Parse(card) else return end
	
	if damage.to and self.player:objectName() ~= damage.to:objectName() and not self:isFriend(damage.to) then
		return card
	end
end

function SmartAI:askForSuddenStrike(player)
	local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local card
	card = self:getCardId("SuddenStrike")
	if card then card = sgs.Card_Parse(card) else return end
	
	if self.player:objectName() ~= player:objectName() and not self:isFriend(player) then
		return card
	end
end

function sgs.getDefense(player)
	local defense = math.min(sgs.getValue(player), player:getHp() * 3)
	if player:getArmor() and not (player:getArmor():inherits("GaleShell") or player:getArmor():inherits("JiaoLiao")) then
		defense = defense + 2
	end
	if not player:getArmor() and player:hasSkill("bazhen") then
		defense = defense + 2
	end
	local m = sgs.masochism_skill:split("|")
	for _, masochism in ipairs(m) do
		if player:hasSkill(masochism) then
			defense = defense + 1
		end
	end
	if player:getArmor() and player:getArmor():inherits("EightDiagram") and player:hasSkill("tiandu") then
		defense = defense + 0.3
	end
	if player:hasSkill("jieming") then
		defense = defense + 1
	end
	if player:getMark("@tied")>0 then
		defense = defense + 1
	end
	if player:hasSkill("qingguo") and player:getHandcardNum()>1 then
		defense = defense + 0.5
	end
	if player:hasSkill("longdan") and player:getHandcardNum()>2 then
		defense = defense + 0.3
	end
	return defense
end

function SmartAI:askForDiscard(reason, discard_num, optional, include_equip)
	local callback = sgs.ai_skill_discard[reason]
	if callback and type(callback) == "function" and callback(self, discard_num, optional, include_equip) then
		return callback(self, discard_num, optional, include_equip)
	end
	if optional then return {} end

	local flag = "h"
	if include_equip and (self.player:getEquips():isEmpty() or not self.player:isJilei(self.player:getEquips():first())) then flag = flag .. "e" end
	local cards = self.player:getCards(flag)
	local to_discard = {}
	cards = sgs.QList2Table(cards)
	local aux_func = function(card)
		local place = self.room:getCardPlace(card:getEffectiveId())
		if place == sgs.Player_Equip then
			if card:inherits("GaleShell") then return -2
			elseif card:inherits("JiaoLiao") then return -2
			elseif card:inherits("JiaSuo") then return -2
			elseif card:inherits("SilverLion") and self.player:isWounded() then return -2
			elseif card:inherits("YitianSword") then return -1
			elseif card:inherits("OffensiveHorse") then return 1
			elseif card:inherits("Weapon") then return 2
			elseif card:inherits("DefensiveHorse") then return 3
			elseif card:inherits("Armor") then return 4 end
		elseif self:hasSkills(sgs.lose_equip_skill) then return 5
		else return 0 end
	end
	local compare_func = function(a, b)
		if aux_func(a) ~= aux_func(b) then return aux_func(a) < aux_func(b) end
		return self:getKeepValue(a) < self:getKeepValue(b)
	end

	table.sort(cards, compare_func)
	for _, card in ipairs(cards) do
		if #to_discard >= discard_num then break end
		if not self.player:isJilei(card) then table.insert(to_discard, card:getId()) end
	end
	
	return to_discard
end

function SmartAI:askForCardChosen(who, flags, reason)
	self.room:output(reason)
	local cardchosen = sgs.ai_skill_cardchosen[string.gsub(reason,"%-","_")]
	local card
	if type(cardchosen) == "function" then
		card = cardchosen(self, who, flags)
	end
	if card then
		return card:getId()
	end

	if self:isFriend(who) then
		if flags:match("j") then
			local tricks = who:getCards("j")
			local lightning, indulgence, supply_shortage
			for _, trick in sgs.qlist(tricks) do
				if trick:inherits("Lightning") then
					lightning = trick:getId()
				elseif trick:inherits("Indulgence") or trick:getSuit() == sgs.Card_Diamond then
					indulgence = trick:getId()
				elseif not trick:inherits("Disaster") then
					supply_shortage = trick:getId()
				end
			end


			if self:hasWizard(self.enemies) and lightning then
				return lightning
			end

			if indulgence and supply_shortage then
				if who:getHp() < who:getHandcardNum() then
					return indulgence
				else
					return supply_shortage
				end
			end

			if indulgence or supply_shortage then
				return indulgence or supply_shortage
			end
		end

		if flags:match("e") then
			local zhangjiao = self.room:findPlayerBySkillName("leiji")
			if who:isWounded() and self:isEquip("SilverLion", who) and (not zhangjiao or self:isFriend(zhangjiao))
				and not self:hasSkills("qixi|duanliang", who) then return who:getArmor():getId() end
			if self:isEquip("GaleShell", who) then return who:getArmor():getId() end
			if self:isEquip("JiaoLiao", who) then return who:getArmor():getId() end
			if self:isEquip("JiaSuo", who) then return who:getWeapon():getId() end
			if self:hasSkills(sgs.lose_equip_skill, who) then
				local equips = who:getEquips()
				if not equips:isEmpty() then
					return equips:at(0):getId()
				end
			end
		end
	else
		if flags:match("e") then
			if who:getWeapon() and who:getWeapon():inherits("Crossbow") then
				for _, friend in ipairs(self.friends) do
					if who:distanceTo(friend) <= 1 then return who:getWeapon():getId() end
				end
			end

			self:sort(self.friends, "hp")
			local friend = self.friends[1]
			if self:isWeak(friend) and who:inMyAttackRange(friend) then
				if who:getWeapon() and who:distanceTo(friend) > 1 then return who:getWeapon():getId() end
				if who:getOffensiveHorse() and who:distanceTo(friend) > 1 then return who:getOffensiveHorse():getId() end
			end

			if who:getDefensiveHorse() then
				for _,friend in ipairs(self.friends) do
					if friend:distanceTo(who) == friend:getAttackRange()+1 then
						return who:getDefensiveHorse():getId()
					end
				end
			end

			if who:getArmor() and self:evaluateArmor(who:getArmor(),who)>3 then
				return who:getArmor():getId()
			end

			if self:isEquip("Monkey", who) then
				return who:getOffensiveHorse():getId()
			end
		end

		if flags:match("j") then
			local tricks = who:getCards("j")
			local lightning
			for _, trick in sgs.qlist(tricks) do
				if trick:inherits("Lightning") then
					lightning = trick:getId()
				end
			end
			if self:hasWizard(self.enemies,true) and lightning then
				return lightning
			end
		end

		if flags:match("e") then
			if who:getArmor() and self:evaluateArmor(who:getArmor(), who)>0
				and not (who:getArmor():inherits("SilverLion") and self:isWeak(who)) then
				return who:getArmor():getId()
			end

			if who:getWeapon() then
				if not (who:hasSkill("xiaoji") and (who:getHandcardNum() >= who:getHp())) and not self:isEquip("YitianSword",who) then
					for _,friend in ipairs(self.friends) do
						if (who:distanceTo(friend) <= who:getAttackRange()) and (who:distanceTo(friend) > 1) then
							return who:getWeapon():getId()
						end
					end
				end
			end

			if who:getOffensiveHorse() then
				if who:hasSkill("xiaoji") and who:getHandcardNum() >= who:getHp() then
				else
					for _,friend in ipairs(self.friends) do
						if who:distanceTo(friend) == who:getAttackRange() and
						who:getAttackRange() > 1 then
							return who:getOffensiveHorse():getId()
						end
					end
				end
			end
		end
		if flags:match("h") then
			if not who:isKongcheng() then
				return -1
			end
		end
	end
	local new_flag = ""
	if flags:match("h") then new_flag = "h" end
	if flags:match("e") then new_flag = new_flag.."e" end
	return self:getCardRandomly(who, new_flag) or who:getCards(flags):first():getEffectiveId()
end