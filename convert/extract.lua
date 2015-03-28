
-- Reads the wiki dump HTML as stdin, tries to split it out into
-- the individual pages for later conversion to Markdown

local pages = {}
local currentPage = nil

for line in io.lines() do
	local hrMatch = line:match("<hr%s+/>")
	local aName = line:match([[<a.+name="([^"]+)".+>]])
	
	if hrMatch then
		currentPage = nil
	elseif aName then
		if currentPage == nil then
			currentPage = aName
			pages[currentPage] = pages[currentPage] or ""
		end
	--[=[elseif line:match[[href="#([^"]+)"]] then
		print(line)
		print(line:gsub([[href="#([^"]+)"]], [[href="%1"]]))
	]=]
	elseif currentPage ~= nil then
		pages[currentPage] = pages[currentPage] .. "\n" .. line
	end
	
end

local localLink = [[href="#%s"]]
local pageLink = [[href="%s"]]

for k, v in pairs(pages) do
	--print(#v, k)
	pages[k] = v:gsub([[href="#([^"]+)"]], function(name)
		if pages[name] then
			return(pageLink:format(name))
		else
			return(localLink:format(name))
		end
	end)
end

--print(next(pages))
