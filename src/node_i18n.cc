/*
 * notes: by srl295
 *  - When in NODE_HAVE_SMALL_ICU mode, ICU is linked against "stub" (null) data
 *     ( stubdata/libicudata.a ) containing nothing, no data, and it's also
 *    linked against a "small" data file which the SMALL_ICUDATA_ENTRY_POINT
 *    macro names. That's the "english+root" data.
 *
 *    If icu_data_path is non-null, the user has provided a path and we assume
 *    it goes somewhere useful. We set that path in ICU, and exit.
 *    If icu_data_path is null, they haven't set a path and we want the
 *    "english+root" data.  We call
 *       udata_setCommonData(SMALL_ICUDATA_ENTRY_POINT,...)
 *    to load up the english+root data.
 *
 *  - when NOT in NODE_HAVE_SMALL_ICU mode, ICU is linked directly with its full
 *    data. All of the variables and command line options for changing data at
 *    runtime are disabled, as they wouldn't fully override the internal data.
 *    See:  http://bugs.icu-project.org/trac/ticket/10924
 */


#include "node_i18n.h"

#if defined(NODE_HAVE_I18N_SUPPORT)

#include <unicode/putil.h>
#include <unicode/udata.h>

#ifdef NODE_HAVE_SMALL_ICU

#ifndef NODE_SPECIAL_ICU_DIR
#define NODE_SPECIAL_ICU_DIR "kittens"
#endif

#ifdef NODE_SPECIAL_ICU_DIR
#include <stdio.h>
/**
 * cheap "access" function.
 *
 * @param s
 * @return nonzero if "s" is readable
 */
static int cheap_access(const char *s) {
  FILE *f = fopen(s, "r");
  if(f) {
    fclose(f);
    printf("yay: %s\n", s);
    return 1;
  } else {
    printf("NAY: %s\n", s);
    return 0;
  }
}
#endif

/* if this is defined, we have a 'secondary' entry point.
   compare following to utypes.h defs for U_ICUDATA_ENTRY_POINT */
#define SMALL_ICUDATA_ENTRY_POINT \
  SMALL_DEF2(U_ICU_VERSION_MAJOR_NUM, U_LIB_SUFFIX_C_NAME)
#define SMALL_DEF2(major, suff) SMALL_DEF(major, suff)
#ifndef U_LIB_SUFFIX_C_NAME
#define SMALL_DEF(major, suff) icusmdt##major##_dat
#else
#define SMALL_DEF(major, suff) icusmdt##suff##major##_dat
#endif

extern "C" const char U_DATA_API SMALL_ICUDATA_ENTRY_POINT[];
#endif

namespace node {
namespace i18n {

bool InitializeICUDirectory(const char* icu_data_path) {
  if (icu_data_path != nullptr) {
    u_setDataDirectory(icu_data_path);
    return true;  // no error
  } else {
    UErrorCode status = U_ZERO_ERROR;
#ifdef NODE_HAVE_SMALL_ICU

#ifdef NODE_SPECIAL_ICU_DIR

    char postfix[2048];
    strcpy(postfix, NODE_SPECIAL_ICU_DIR); // kittens
    strcat(postfix, U_FILE_SEP_STRING);    // /
    strcat(postfix, U_ICUDATA_NAME);       // icudt23e
    strcat(postfix, ".dat");               // .dat
    puts(postfix);

    if(cheap_access(postfix)) {
      u_setDataDirectory(postfix);
      return (status == U_ZERO_ERROR);
    }

    char dir[2048];
    strcpy(dir, "/usr/local/");
    strcat(dir, postfix);
    if(cheap_access(dir)) {
      u_setDataDirectory(dir);
      return (status == U_ZERO_ERROR);
    }      
    
#endif    
    // install the 'small' data.
    udata_setCommonData(&SMALL_ICUDATA_ENTRY_POINT, &status);
#else  // !NODE_HAVE_SMALL_ICU
    // no small data, so nothing to do.
#endif  // !NODE_HAVE_SMALL_ICU
    return (status == U_ZERO_ERROR);
  }
}

}  // namespace i18n
}  // namespace node

#endif  // NODE_HAVE_I18N_SUPPORT
