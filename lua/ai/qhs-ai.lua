sgs.ai_use_value =
{

--skill cards
	FenbeihCard = 4.4,
	FangshuhCard = 8.5,
	ZhixinhCard = 9.2,
	FenlihCard = 9.2,
	ZhenluanhCard = 8.5,
	XianxihCard = 3.5,
	GuijihCard = 8.5,
	JunlinghCard = 6.7,
	CaijianhCard = 6.5,
	
--normal cards
}

sgs.ai_use_priority = {
--priority of using an active card

--skill cards
	FenbeihCard = 6.4,
	FangshuhCard = 8.5,
	ZhixinhCard = 9.2,
	FenlihCard = 9.2,
	ZhenluanhCard = 8.5,
	XianxihCard = 3.5,
	GuijihCard = 8.5,
	JunlinghCard = 6.7,
	CaijianhCard = 6.5,
}


sgs.ai_chaofeng = {
	huatuo = 6,
	sunshangxiang = 6,
	wisjiangwan = 6,

	erzhang = 5,
	wisshuijing = 5,

	zuocih = 4,
	
	xinxianyingh = 3,
	zhangxiuh = 3,
	
	simahuih = 2,
	ganfurenh = 2,
	wangyih = 2,
	
	kongrongh = 1,
	huangfusongh = 1,
	handangh = 1,
	wangyunh = 1,
	guohuaih = 1,
	zhangchunhuah = 1,
		
	chendaoh = 0,

	caozhangh = -1,

	mayunluh = zhaoyun,
}

--shuijiingh
sgs.ai_skill_invoke.shuijingh = function(self, data)
	local target = room:getTag("ShuijinghTarget"):toPlayer()
	if self:isEnemy(target) then
		return true
	else
		return false
	end
end

--fenbeih
local fenbeih_skill = {}
fenbeih_skill.name = "fenbeih"
table.insert(sgs.ai_skills, fenbeih_skill)
fenbeih_skill.getTurnUseCard=function(self)
	if not self.player:hasUsed("FenbeihCard") then 
		return sgs.Card_Parse("@FenbeihCard=.") 
	end
end

sgs.ai_skill_use["@@fenbeih"]=function(self,prompt)
	for _,enemy in ipairs(self.enemies) do
		if enemy:getHandcardNum() >= 2 then
			return "@FenbeihCard=.->"..enemy:objectName()
		end
	end
	return "."
end


