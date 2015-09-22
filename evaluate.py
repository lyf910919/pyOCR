import sys
import os

def getWordScore(a, b):
	n, m = len(a), len(b)
	if n == 0 or m == 0:
		return 0
	score = [[0 for i in range(m)] for i in range(n)]
	for i in range(n):
		score[i][0] = int(a[i] == b[0])
	for j in range(m):
		score[0][j] = int(a[0] == b[j])

	for i in range(1, n):
		for j in range(1, m):
			score[i][j] = max(score[i-1][j], score[i][j-1], score[i-1][j-1]\
				+ int(a[i] == b[j]))
	return float(score[n-1][m-1]) / max(n,m)

def getScore(sa, sb):
	aList = sa.split(' ')
	bList = sb.split(' ')
	n, m = len(aList), len(bList)

	score = [[0 for i in range(m)] for i in range(n)]
	for i in range(n):
		score[i][0] = getWordScore(aList[i], bList[0])
	for j in range(m):
		score[0][j] = getWordScore(aList[0], bList[j])

	for i in range(1, n):
		for j in range(1, m):
			score[i][j] = max(score[i-1][j], score[i][j-1], score[i-1][j-1]\
				+ int(getWordScore(aList[i], bList[j])))
	return float(score[n-1][m-1]) / max(n,m)

def main():
	with open(sys.argv[1], 'r') as f:
		stringList1 = f.read().split('\n')
	
	stringList2 = []
	folder = './text2/';
	for i in range(1, 201):
		with open(folder + str(i), 'r') as f:
			stringList2.append(f.read().strip())

	stringList1 = filter(lambda x: len(x) > 0, stringList1)
	stringList2 = filter(lambda x: len(x) > 0, stringList2)
	if len(stringList1) != len(stringList2):
		raise AssertionError('Two lists\' length not match: %s %s' \
		 % (len(stringList1), len(stringList2)))
	# with open('label.txt', 'w+') as f:
	# 	for text in stringList2:
	# 		print >> f, text
	scoreList = []
	scoreListWord = []
	for i in range(len(stringList1)):
		print getScore(stringList1[i], stringList2[i])
		scoreList.append(getScore(stringList1[i], stringList2[i]))
		scoreListWord.append(getWordScore(stringList1[i], stringList2[i]))
	with open(sys.argv[2], 'w+') as f:
		for i in range(len(scoreList)):
			if scoreList[i] < 1:
				print >> f, stringList1[i]
				print >> f, stringList2[i]
				print >> f, scoreList[i], scoreListWord[i]
		print >> f, 'average word score:', sum(scoreList) / len(scoreList)
		print >> f, 'average char score:', sum(scoreListWord) / len(scoreListWord)

if __name__ == '__main__':
	main()