# coding: utf-8
import urllib2
import urllib
import time
import pickle
import codecs
import os
import sys
import string
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
		html = response.read()
	except:
		print 'cannot read from google'
		return ' '.join(s)

	#get correction
	soup = BeautifulSoup(html, 'html.parser')
	found = soup.find_all('a', class_='spell')
	# found = soup.find_all('h3', class_='r')
	# for line in found:
	# 	print line
	if len(found) == 0:
		return ' '.join(s)
	correction = found[0].get_text()
	if len(correction) == 0:
		correction = ' '.join(s)
	return correction

def getScore(src, dst):
	n, m = len(src), len(dst)
	a, b = src, dst
	if n == 0 or m == 0:
		return 0
	score = [[0 for i in range(m)] for j in range(n)]
	for i in range(n):
		score[i][0] = int(a[i] == b[0])
	for j in range(m):
		score[0][j] = int(a[0] == b[j])

	for i in range(1, n):
		for j in range(1, m):
			score[i][j] = max(score[i-1][j], score[i][j-1], score[i-1][j-1]\
				+ int(a[i] == b[j]))
	return float(score[n-1][m-1]) / n

def match(strSrc, strDst):
	punct = string.punctuation.replace('.', '')
	SCORE_THRESH = 0.7
	if type(strSrc) == unicode and type(strDst) != unicode:
		strDst = unicode(strDst)
	if type(strSrc) != unicode and type(strDst) == unicode:
		print type(strSrc)
		strSrc = unicode(strSrc)
	srcList = strSrc.split(' ')
	dstList = strDst.split(' ')
	for i in range(len(srcList)):
		maxScore, maxInd = 0, 0
		for j in range(len(dstList)):
			while len(dstList[j]) > 0 and dstList[j][0] != srcList[i][0] and\
			dstList[j][0] in punct:
				dstList[j] = dstList[j][1:]
			while len(dstList[j]) > 0 and dstList[j][-1] != srcList[i][-1] and\
			dstList[j][-1] in punct:
				dstList[j] = dstList[j][:-1]
			score = getScore(srcList[i], dstList[j])
			if score > maxScore:
				maxScore = score
				maxInd = j
		print srcList[i], ':', maxScore
		if maxScore > SCORE_THRESH:
			srcList[i] = dstList[maxInd]
			print srcList[i]
	return ' '.join(srcList)

def correct2(s):
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
		html = response.read()
	except:
		print 'cannot read from google'
		return ' '.join(s)

	#get correction
	soup = BeautifulSoup(html, 'html.parser')
	#found = soup.find_all('a', class_='spell')
	found = soup.find_all('h3', class_='r')
	# for line in found:
	# 	print line
	if len(found) == 0:
		return ' '.join(s)
	topNum = max(4, len(found))
	topResults = ' '.join([found[i].get_text() for i in range(topNum)])
	correction = match(' '.join(s), topResults)
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
			if type(word) == unicode:
				word = word.encode('utf8')
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
	#main()
	with open(sys.argv[1], 'r') as f:
		wordList = f.read().split('\n')
	# with open(sys.argv[2], 'r') as f:
	# 	googleList = f.read().split('\n')
	for i in range(len(wordList)):
		correction = correct2(wordList[i].strip())
		print correction
		wordList[i] = correction
	with open(sys.argv[2], 'w+') as f:
		for word in wordList:
			if type(word) == unicode:
				word = word.encode('utf8')
			print >> f, word