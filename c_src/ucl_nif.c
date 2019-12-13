#include <stdint.h>
#include <strings.h>
#include <stdlib.h>
#include <malloc_np.h>
#include "erl_nif.h"
#include "ucl.h"

    static ERL_NIF_TERM
to_json(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    // buffer for returning parsed JSON
    unsigned char* buf;
    ERL_NIF_TERM json;

    // ucl bits and pieces
    struct ucl_parser *parser = NULL;

    ucl_object_t *ucl = NULL;
    /* ucl_emitter_t emitter = UCL_EMIT_JSON; */

    if (!enif_is_binary(env, argv[0])) {
        ERL_NIF_TERM error = enif_make_atom(env, "error");
        ERL_NIF_TERM reason = enif_make_atom(env, "badarg");
        return enif_make_tuple2(env, error, reason);
    }

    ErlNifBinary binary;
    enif_inspect_binary(env, argv[0], &binary);

    // TODO ensure binary is NULL terminated

    // parse UCL
    // https://github.com/vstakhov/libucl/blob/master/doc/api.md#parser-functions-1
    parser = ucl_parser_new(UCL_PARSER_KEY_LOWERCASE);
    ucl_parser_add_chunk (parser, binary.data, binary.size);

    if (ucl_parser_get_error (parser)) {
        ERL_NIF_TERM error = enif_make_atom(env, "error");
        ERL_NIF_TERM reason = enif_make_atom(env, "ucl_invalid");
        return enif_make_tuple2(env, error, reason);
    }
    else if ((ucl = ucl_parser_get_object(parser)) == NULL) {
        ERL_NIF_TERM error = enif_make_atom(env, "error");
        ERL_NIF_TERM reason = enif_make_atom(env, "ucl_no_root");
        return enif_make_tuple2(env, error, reason);
    }
    else {
        ucl = ucl_parser_get_object (parser);
    }

    if (parser != NULL) {
        ucl_parser_free (parser);
    }
    if (ucl != NULL) {
        ucl_object_unref (ucl);
    }

    // return parsed JSON
    ERL_NIF_TERM ok = enif_make_atom(env, "ok");
    buf = enif_make_new_binary(env, binary.size, &json);
    bzero(buf, binary.size);

    return enif_make_tuple2(env, ok, json);
}

static ErlNifFunc nif_funcs[] = {
    {"to_json", 1, to_json}
};

ERL_NIF_INIT(ucl, nif_funcs, NULL, NULL, NULL, NULL)
