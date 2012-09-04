taps = 10;
f = [0,567-200,567-30,567+30,567+200,1000]/1000;
a = [1,1,0,0,1,1];
b = firgr(taps,f,a,'minphase');
b = -b(2:taps+1)/b(1);
b'
%fvtool(b,1);
