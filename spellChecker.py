# coding: utf-8
import urllib2
import urllib
import time
import pickle
import codecs
import os
import sys
from bs4 import BeautifulSoup

def correct(s):
	#construct query term
	s = s.split(' ')
	q = ""
	for i in range(len(s)):
		q += s[i] + '+'
	q = q[:-1]

	#search google
	dic = {'user-agent': 'Mozilla/5.0 (Windows NT 6.1; WOW64; rv:38.0) Gecko/20100101 Firefox/38.0',\
	'referer': 'http://www.google.com/'}
	url = 'https://www.google.com/search?biw=1100&bih=625&noj=1&q=%s&oq=%s'%(q,q)
	request = urllib2.Request(url, None, dic)
	try:
		response = urllib2.urlopen(request, timeout = 10)
		html = response.read().decode('utf-8')
	except:
		print 'cannot read from google'
		return ' '.join(s)

	#get correction
	soup = BeautifulSoup(html, 'html.parser')
	found = soup.find_all('a', class_='spell')
	if len(found) == 0:
		return ' '.join(s)
	correction = found[0].get_text()
	if len(correction) == 0:
		correction = ' '.join(s)
	return correction



def main():
	with open(sys.argv[1], 'r') as f:
		wordList = f.read().split('\n')
	for i in range(len(wordList)):
		correction = correct(wordList[i].strip())
		print correction
		time.sleep(1)
		wordList[i] = correction
	with open(sys.argv[2], 'w+') as f:
		for word in wordList:
			print >> f, word

	'''
	dic = {'user-agent': 'Mozilla/5.0 (Windows NT 6.1; WOW64; rv:38.0) Gecko/20100101 Firefox/38.0',\
	'referer': 'http://www.google.com/'}
	q = ""
	for i in range(1, len(sys.argv)):
		q += sys.argv[i] + '+'
	q = q[:-1]
	print q
	url = 'https://www.google.com/search?biw=1100&bih=625&noj=1&q=%s&oq=%s'%(q,q)
	request = urllib2.Request(url, None, dic)
	response = urllib2.urlopen(request, timeout = 10)
	html = response.read().decode('utf-8')
	soup = BeautifulSoup(html, 'html.parser')
	print soup.find_all('a', class_='spell')[0].get_text()
	print soup.find_all('h3', class_='r')[0].get_text()
	with open('google.html', 'w+') as f:
		print >> f, soup.prettify().encode('utf-8')
	'''
	

if __name__ == '__main__':
	main()