./csim-ref -v -s 1 -E 1 -b 1 -t traces/yi2.trace > _case1_ans;
./csim-ref -v -s 4 -E 2 -b 4 -t traces/yi.trace > _case2_ans;
./csim-ref -v -s 2 -E 1 -b 4 -t traces/dave.trace > _case3_ans;
./csim-ref -v -s 5 -E 1 -b 5 -t traces/long.trace > _case4_ans;

./csim-ref -v -s 2 -E 1 -b 3 -t traces/trans.trace > _case5_ans;
./csim-ref -v -s 2 -E 2 -b 3 -t traces/trans.trace > _case6_ans; 
./csim-ref -v -s 2 -E 4 -b 3 -t traces/trans.trace > _case7_ans;
./csim-ref -v -s 5 -E 1 -b 5 -t traces/trans.trace > _case8_ans;