/** .root MyCpp library
 * .title Base MyNC library.
 * .abstract Portability, refcounting, containers, I/O, and more.
 *
 * <toc>
 *   <cat anybase/>
 * </toc>
 */

/** .cat anybase
 * .title Conventions on inheritance for basic templates
 *
 * .desc
 * These inheritance rules are somewhat awkward. There's little reason for
 * them, but sometimes they are convenient. Following these rules should not
 * bring any additional overhead except for a few more class names.
 *
 * Templates with "_common" prefix contain the most generic constructs, which
 * may be of interest for external world.
 *
 * Templates with "_anybase" prefix can be used for passing basic objects as
 * function parameters.
 *
 * Templates without any prefix have a base class as their last parameter.
 * This makes it possible to inherit from Object and SimplyReferenced for
 * convenience.
 *
 * Non-base templates are provided in two flavors:
 * 1) Templates with arbitrary base as a parameter. Names of such templates
 *    end with an underscore;
 * 2) Typedefed non-templates without an underscore at the end of their names.
 *    Hard-coded to inherit from EmptyBase.
 */

