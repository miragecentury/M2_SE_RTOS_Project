;/**************************************************************************//**
; * @file     SAMV71_GPNVM.s
; * @brief    General-purpose Non-volatile Memory Bits for
; *           Atmel SAMV71 Device Series
; * @version  V1.00
; * @date     23. January 2015
; *
; * @note
; *
; ******************************************************************************/
;/* Copyright (c) 2011 - 2015 ARM LIMITED
;
;   All rights reserved.
;   Redistribution and use in source and binary forms, with or without
;   modification, are permitted provided that the following conditions are met:
;   - Redistributions of source code must retain the above copyright
;     notice, this list of conditions and the following disclaimer.
;   - Redistributions in binary form must reproduce the above copyright
;     notice, this list of conditions and the following disclaimer in the
;     documentation and/or other materials provided with the distribution.
;   - Neither the name of ARM nor the names of its contributors may be used
;     to endorse or promote products derived from this software without
;     specific prior written permission.
;   *
;   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
;   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
;   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
;   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
;   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
;   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
;   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
;   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
;   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
;   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
;   POSSIBILITY OF SUCH DAMAGE.
;   ---------------------------------------------------------------------------*/
;/*
;//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
;*/


;// <e> General-purpose Non-volatile Memory Bits
GPNVM       EQU     1

;//   <o0.0> Security bit
;//   <o0.1> Boot mode selection
;//          <0=> Boot from ROM
;//          <1=> Boot from Flash
;//   <o0.7..8> TCM configuration
;//          <0=>   0KB DTCM + 0KB ITCM
;//          <1=>  32KB DTCM + 32KB ITCM
;//          <2=>  64KB DTCM + 64KB ITCM
;//          <3=> 128KB DTCM + 128KB ITCM
GPNVM_BITS      EQU     0x002

;// </e>

                IF      GPNVM <> 0
                AREA    |.ARM.__AT_0x1FFFFFF0|, CODE, READONLY
                DCD     GPNVM_BITS
                ENDIF

                END
