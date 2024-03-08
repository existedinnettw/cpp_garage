# CSP libs benchmark

Both benchmark show about **15us** cpu time for per ping pong(in wsl).
I think context swtich is the main reason slow. if both ping and pong take 2 times, it take may take 2*5us(expeced) each.

[How long does it take to make a context switch?](https://www.google.com/search?client=firefox-b-d&sca_esv=3add410d07b2a008&sxsrf=ACQVn0-ukJ4XV3IpR4UJPal6g15R2-o5Pw:1709823630284&q=context+switch+micro+second+on+linux&tbm=isch&source=lnms&sa=X&ved=2ahUKEwiygvKwteKEAxXoVPUHHVeVB5IQ0pQJegQICxAB&biw=1920&bih=919#imgrc=GlUhlIiB-xlJWM)

我用這個測的結果
[Comparison of Rust async and Linux thread context switch time and memory use](https://github.com/jimblandy/context-switch/tree/master)
thread 居然要20us, context 2us，可能是wsl 的關係.
換到linux 下後確實 thread context switch 約6.97us，async 約5.77us。差20%。github 上是差50%。

換到pure linux sys，pingpong 1 次約5~6us, -> 1次context switch 大概2.5us。

後來用了 [async-chan](https://github.com/MiSo1289/asiochan)，可以減少一半，context switch，性能很好。
