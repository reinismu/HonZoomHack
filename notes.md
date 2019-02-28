## Notes

### ZoomOut edit
ZoomOut + 0x4f (minss   xmm1, xmm2) -> (movss   xmm1, xmm2)
ZoomOut + 0x5e (minss   xmm1, xmm3) -> (movss   xmm1, xmm3)

PrepareClientState + 0x222 (movss   cs:currentCammeraZoom, xmm0) -> (NOP, NOP ..)


PrepareClientState + 0xc78 (minss   xmm2, xmm3 ) -> (movss   xmm2, xmm3)

0x7fab30514220 <-- player zoom location

