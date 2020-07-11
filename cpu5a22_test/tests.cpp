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
    REQUIRE(bus->cpu.a.w == 0x39);
    REQUIRE(bus->cpu.y.w == 0xdd);
    REQUIRE(bus->cpu.x.w == 0xef);
  }
}

TEST_CASE("CMP", "[cmp]") {
  auto bus = run(R"(
    lda $00
    cmp #$a5
    bne $02
    ldx #$ff
  )");

  SECTION("cmp") {
    REQUIRE(bus->cpu.x.w == 0xff);
  }
}

TEST_CASE("65816 switch", "[65816]") {
  auto bus = run(R"(
    sec
    xce
  )");

  SECTION("native mode transition") {
    REQUIRE(bus->cpu.native);
  }
}