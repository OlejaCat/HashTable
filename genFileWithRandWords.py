import random

random.seed(228)

NUM_OF_WORDS = int(2e5)
MIN_WORD_LEN = 15
MAX_WORD_LEN = 25

with open("randWordsFile.txt", "w") as f:
    for _ in range(NUM_OF_WORDS):
        wordLen = random.randint(MIN_WORD_LEN, MAX_WORD_LEN)
        word = "".join([chr(ord('a') + random.randint(0, 25)) for _ in range(wordLen)])
        word +=  '\n'
        f.write(word)
