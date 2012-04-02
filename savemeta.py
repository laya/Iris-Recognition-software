import sqlite3
import re
import urllib
import json

fields = ["name", "image", "image_size", "border", "alt", "caption", "director", "producer", "writer", "screenplay", "story", "based_on", "narrator", "starring", "music", "cinematography", "editing", "studio", "distributor", "released", "runtime", "country", "language", "budget", "gross"]

base = "http://en.wikipedia.org/w/api.php?action=parse&format=json&prop=wikitext&page="

moviesfile = open("movies.txt", "r")

def storeData(data):
	''' SQLite writer '''
	conn = sqlite3.connect("/tmp/movies")
	c = conn.cursor()
	createstr = 'create table movies ('+",".join([field for field in fields])+")"
	try:
		c.execute(createstr)
	except:
		pass
	vals = []
	for field in fields:
		if field.replace("_", " ") in data.keys():
			vals.append("'"+data[field.replace("_"," ")]+"'")
		else:
			vals.append("''")

	c.execute("insert into movies values ("+",".join(vals)+")")
	# Save (commit) the changes
	conn.commit()
	c.execute("select * from movies")
	for row in c:
		print row
	# We can also close the cursor if we are done with it
	c.close()

def parseWikitext(text):
	''' Parse wikitext and return the dict of fields'''
	ob = json.loads( text )
	wktext = ob["parse"]["wikitext"]["*"]
	extractorRegex = re.compile( "{{Infobox film\s*(\s*\|\s*(.*)=(.*)\s*)+\s*}}")
	matches = re.search(extractorRegex, wktext)
	tags = {}
	if matches:
        	parts = matches.group().split('\n')
        	for i in range(len(parts)):
			if "=" not in parts[i]:
				continue
			tag = parts[i].split('=')
			unwanted = ["[[","]]","{{","}}"]
			repcomma = ["|","<br>"]
			for u in unwanted:
				tag[1] = tag[1].replace(u, "")
			for r in repcomma:
				tag[1] = tag[1].replace(r, ",")
			if "<!--" in tag[1]:
				tag[1] = ""
			tags[tag[0].replace("|","").strip()] = tag[1].strip()
	return tags

def requestData( url ):
	''' requests the api for wikitext '''
	pag = urllib.urlopen( base+url )
	return pag.read()


	
for title in moviesfile.readlines():
	wikitext = requestData(title)
	objdict = parseWikitext(wikitext)
	print "\n\n"+title
	print "\n".join(["%s = %s" % (k, v) for k, v in objdict.items()])
	storeData(objdict)	


