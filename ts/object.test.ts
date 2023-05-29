import { expect, test } from "vitest";
import { StringObject } from "./object";

test("string hash key", () => {
  const hello1 = new StringObject("Hello World");
  const hello2 = new StringObject("Hello World");
  const diff1 = new StringObject("My name is johnny");
  const diff2 = new StringObject("My name is johnny");

  expect(hello1.hashCode()).toBe(hello2.hashCode());
  expect(diff1.hashCode()).toBe(diff2.hashCode());
  expect(hello1.hashCode()).not.toBe(diff1.hashCode());
});
