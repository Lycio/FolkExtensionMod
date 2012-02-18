--rengong

--shishi
sgs.ai_skill_invoke.shishi = true

--feiyan
sgs.ai_skill_invoke.feiyan = function(self,data)
	local effect = data:toEffect()
	return self:isEnemy(effect.from)
end

--zhenhuo
sgs.ai_skill_invoke["@zhenhuo"]=function(self,data)
	local effect = data:toEffect()
	return self:isEnemy(effect.to)
end

--guishu
sgs.ai_skill_invoke["@guishu"]=function(self,prompt)
    local judge = self.player:getTag("Judge"):toJudge()
	
	if self:needRetrial(judge) then
		local all_cards = self.player:getCards("he")
		local cards = {}
		for _, card in sgs.qlist(all_cards) do
			if card:isRed() then
				table.insert(cards, card)
			end
		end
		
		local card_id = self:getRetrialCardId(cards, judge)
		if card_id ~= -1 then
			return "@GuishuCard=" .. card_id
		end
	end
	
	return "."
end

--chengyuanzhi's baonue
sgs.ai_skill_invoke.huangjinbaonue = function(self, data)
	local damage = data:toDamage()
	return self:isEnemy(damage.to)
end

--jieliang
sgs.ai_skill_invoke.jieliang = function(self, data)
	local damage = data:toDamage()
	return self:isEnemy(damage.to) and not damage.to:isWeak()
end

--leichui
sgs.ai_skill_invoke.leichui = function(self, data)
	local damage = data:toDamage()
	return self:isEnemy(damage.to)
end

--xiuzhen
sgs.ai_skill_invoke.xiuzhen = function(self, data)
	local damage = data:toDamage()
	return not self:isFriend(damage.from)
end
