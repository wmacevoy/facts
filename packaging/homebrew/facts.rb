class Facts < Formula
  desc "Lightweight, beginner-friendly C/C++ testing library"
  homepage "https://github.com/wmacevoy/facts"
  url "https://github.com/wmacevoy/facts/archive/refs/tags/v2.3.0.tar.gz"
  # Update sha256 after creating the release tarball:
  #   shasum -a 256 v2.3.0.tar.gz
  sha256 "548f234accbd0647f3086c054cac3c9e003d33afa2725edddf89fa4c24f3d3f4"
  license "MIT"

  depends_on "cmake" => :build

  def install
    system "cmake", "-B", "build", *std_cmake_args
    system "cmake", "--build", "build"
    system "cmake", "--install", "build"
  end

  test do
    # Test C library
    (testpath/"test_c.c").write <<~C
      #include <facts.h>
      FACTS(Smoke) {
        int a = 1, b = 1;
        FACT(a, ==, b);
      }
      FACTS_REGISTER_ALL() { FACTS_REGISTER(Smoke); }
      FACTS_MAIN
    C
    system ENV.cc, "test_c.c", "-std=c11",
           "-I#{include}", "-L#{lib}", "-lfacts", "-o", "test_c"
    system "./test_c"

    # Test C++ library
    (testpath/"test_cpp.cpp").write <<~CXX
      #include <facts.h>
      FACTS(Smoke) {
        int a = 1, b = 1;
        FACT(a, ==, b);
      }
      FACTS_REGISTER_ALL() { FACTS_REGISTER(Smoke); }
      FACTS_MAIN
    CXX
    system ENV.cxx, "test_cpp.cpp", "-std=c++11",
           "-I#{include}", "-L#{lib}", "-lfacts++", "-o", "test_cpp"
    system "./test_cpp"
  end
end
