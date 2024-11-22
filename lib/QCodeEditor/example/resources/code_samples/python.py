import numpy

def sqr_array(x):
  '''
  Documentation for sqr_array.
  A multi line string!'
  '''
  
  return np.array(x)**2

s = 'this is a string'
d = 10
x = np.arange(0,d,d/10)
y = sqr_array(x)
