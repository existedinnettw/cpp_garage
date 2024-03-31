# CSP libs benchmark

- test your machine context swtich
  - [Comparison of Rust async and Linux thread context switch time and memory use](https://github.com/jimblandy/context-switch/tree/master)
    - 確實 thread context switch 約6.97us，async 約5.77us。差20%。github 上是差50%。
    - [How long does it take to make a context switch?](https://www.google.com/search?client=firefox-b-d&sca_esv=3add410d07b2a008&sxsrf=ACQVn0-ukJ4XV3IpR4UJPal6g15R2-o5Pw:1709823630284&q=context+switch+micro+second+on+linux&tbm=isch&source=lnms&sa=X&ved=2ahUKEwiygvKwteKEAxXoVPUHHVeVB5IQ0pQJegQICxAB&biw=1920&bih=919#imgrc=GlUhlIiB-xlJWM)
- wsl thread context switch will be extremely slow, about **15us** cpu time for per ping pong.
- coroutine may speed up a lot.
  - 後來用了 [async-chan](https://github.com/MiSo1289/asiochan)，可以減少一半 context switch 時間 (2.5us->1us)，性能很好。
  - 大家一般認為 coroutine switch time 大概就 context switch 1/10，所以要確定為什麼沒達到。e.g. coroutine數目要夠多?
