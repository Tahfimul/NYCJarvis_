file(REMOVE_RECURSE
  "libasio.a"
  "libasio.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/asio.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
