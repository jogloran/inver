#include "catch.hpp"
#include "utils.hpp"

TEST_CASE("LDA", "[lda]") {
  auto bus = run(R"(
    lda #$39
    ldx #$fc
    ldy #$dd
    tyx
    ldx #$ef
  )");

  SECTION("state correct") {
    REQUIRE(bus->cpu->a.w == 0x39);
    REQUIRE(bus->cpu->y.w == 0xdd);
    REQUIRE(bus->cpu->x.w == 0xef);
  }
}

TEST_CASE("CMP", "[cmp]") {
  auto bus = run(R"(
    lda $00
    cmp #$a5
    bne DONE
    ldx #$ff
DONE nop
  )");

  SECTION("cmp") {
    REQUIRE(bus->cpu->x.w == 0xff);
  }
}

TEST_CASE("65816 switch", "[65816]") {
  auto bus = run(R"(
    clc
    xce
  )");

  SECTION("native mode transition") {
    REQUIRE(bus->cpu->e == 0);
  }
}

TEST_CASE("XBA", "[xba]") {
  auto bus = run(R"(
    clc
    xce
    !al
    lda #$fedc
    xba
    tax
    lda #$fedc
    tay
  )");

  SECTION("XBA") {
    REQUIRE(bus->cpu->x.w == 0xdcfe);
    REQUIRE(bus->cpu->y.w == 0xfedc);
  }
}

TEST_CASE("width", "[width]") {
  auto bus = run(R"(
    clc
    xce
    rep #$30
  )");

  SECTION("REP") {
    REQUIRE(!bus->cpu->p.x);
    REQUIRE(!bus->cpu->p.m);
  }

  auto bus2 = run(R"(
    clc
    xce
    sep #$30
  )");

  SECTION("REP") {
    REQUIRE(bus2->cpu->p.x);
    REQUIRE(bus2->cpu->p.m);
  }
}

TEST_CASE("LSR", "[lsr]") {
  SECTION("LSR") {
    auto bus = run(R"(
      clc
      xce
      sep #$30
      lda #$abcd
      lsr
      tax
      lsr
      tay
      lsr
    )");

    REQUIRE(bus->cpu->x.w == 0x55e6);
    REQUIRE(bus->cpu->y.w == 0x2af3);
    REQUIRE(bus->cpu->a.w == 0x1579);
  }
}

TEST_CASE("addressing", "[addr]") {
  SECTION("ABSOLUTE") {
    auto bus = run(R"(
      clc
      xce
      sep #$30
      !al
      jmp ok
      nop
      nop
      nop
      nop
ok    ldx #$ee
      jmp done
      ldx #$ff
done  nop
    )");

    REQUIRE(bus->cpu->x.w == 0xee);
  }

  SECTION("ABSOLUTE,X AND ABSOLUTE,Y") {

  }

  SECTION("(ABSOLUTE) AND [ABSOLUTE]") {

  }

  SECTION("(ABSOLUTE,X)") {

  }

  SECTION("ACCUMULATOR") {

  }

  SECTION("DIRECT") {

  }

  SECTION("DIRECT,X AND DIRECT,Y") {

  }

  SECTION("(DIRECT)") {

  }

  SECTION("[DIRECT]") {

  }

  SECTION("(DIRECT,X)") {

  }

  SECTION("(DIRECT),Y") {

  }

  SECTION("[DIRECT],Y") {

  }

  SECTION("IMMEDIATE") {

  }

  SECTION("IMPLIED") {

  }

  SECTION("LONG") {

  }

  SECTION("LONG,X") {

  }

  SECTION("RELATIVE8 AND RELATIVE16") {

  }

  SECTION("SOURCE,DESTINATION") {

  }

  SECTION("STACK,S") {

  }

  SECTION("(STACK,S),Y") {

  }

}