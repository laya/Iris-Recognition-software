import json
import HTMLParser, urllib
import sys  

class linkParser(HTMLParser.HTMLParser):
    def __init__(self):
        HTMLParser.HTMLParser.__init__(self)
        self.links = []
    def handle_starttag(self, tag, attrs):
        if tag=='a':
            self.links.append(dict(attrs)['href'])

base = "http://en.wikipedia.org/w/api.php?action=query&list=categorymembers&format=json&cmtitle=Category%3ATamil-language%20films&cmprop=title"


def getFilms( contstr ):
	try:
		url = base+"&cmcontinue="+contstr.encode("utf-8")
		src = urllib.urlopen(url)
		return src.read()
	except:
		return None

def extractNames( src ):
	obj = json.loads( src )
	query = obj["query"]
	query_cont = obj["query-continue"]
	mems = query["categorymembers"]
	category_mem = query["categorymembers"]
	conts = obj["query-continue"]["categorymembers"]["cmcontinue"] 
	for member in category_mem:
		try:
			fd.write( member["title"] )
			fd.write( "\n" )
		except:
			pass
	return conts
	save
htmlSource = urllib.urlopen(base)
fd = open('movies.txt','w+')
conti = extractNames( htmlSource.read() )

while conti is not None:
	newsrc = getFilms(conti)
	if newsrc is not None:
		conti = extractNames( newsrc )
fd.close() 
